
# Open source copyright declaration based on BSD open source template:
# http://www.opensource.org/licenses/bsd-license.php
#
# This file is part of the OPS distribution.
#
# Copyright (c) 2013, Mike Giles and others. Please see the AUTHORS file in
# the main source directory for a full list of copyright holders.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# The name of Mike Giles may not be used to endorse or promote products
# derived from this software without specific prior written permission.
# THIS SOFTWARE IS PROVIDED BY Mike Giles ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL Mike Giles BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""
OPS MPI_seq code generator

This routine is called by ops.py which parses the input files

It produces a file xxx_seq_kernel.cpp for each kernel,
plus a master kernel file

"""

import re
import datetime
import os

def comm(line):
  global file_text, FORTRAN, CPP
  global depth
  prefix = ' '*depth
  if len(line) == 0:
    file_text +='\n'
  else:
    file_text +=prefix+'//'+line+'\n'

def code(text):
  global file_text, g_m
  global depth
  prefix = ''
  if len(text) != 0:
    prefix = ' '*depth

  file_text += prefix+text+'\n'

def FOR(i,start,finish):
  global file_text
  global depth
  code('for ( int '+i+'='+start+'; '+i+'<'+finish+'; '+i+'++ ){')
  depth += 2

def FOR2(i,start,finish,increment):
  global file_text
  global depth
  code('for ( int '+i+'='+start+'; '+i+'<'+finish+'; '+i+'+='+increment+' ){')
  depth += 2

def WHILE(line):
  global file_text
  global depth
  code('while ( '+ line+ ' ){')
  depth += 2

def ENDWHILE():
  global file_text
  global depth
  depth -= 2
  code('}')

def ENDFOR():
  global file_text
  global depth
  depth -= 2
  code('}')

def IF(line):
  global file_text
  global depth
  code('if ('+ line + ') {')
  depth += 2

def ELSEIF(line):
  global file_text
  global depth
  code('else if ('+ line + ') {')
  depth += 2

def ELSE():
  global file_text
  global depth
  code('else {')
  depth += 2

def ENDIF():
  global file_text
  global depth
  depth -= 2
  code('}')

def mult(text, i, n):
  text = text + '1'
  for nn in range (0, i):
    text = text + '* args['+str(n)+'].dat->size['+str(nn)+']'

  return text

def para_parse(text, j, op_b, cl_b):
    """Parsing code block, i.e. text to find the correct closing brace"""

    depth = 0
    loc2 = j

    while 1:
      if text[loc2] == op_b:
            depth = depth + 1

      elif text[loc2] == cl_b:
            depth = depth - 1
            if depth == 0:
                return loc2
      loc2 = loc2 + 1
      
def comment_remover(text):
    """Remove comments from text"""

    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return ''
        else:
            return s
    pattern = re.compile(
        r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
        re.DOTALL | re.MULTILINE
    )
    return re.sub(pattern, replacer, text)

def remove_trailing_w_space(text):
  line_start = 0
  line = ""
  line_end = 0
  striped_test = ''
  count = 0
  while 1:
    line_end =  text.find("\n",line_start+1)
    line = text[line_start:line_end]
    line = line.rstrip()
    striped_test = striped_test + line +'\n'
    line_start = line_end + 1
    line = ""
    if line_end < 0:
      return striped_test

def parse_signature(text):
  text2 = text.replace('const','')
  text2 = text2.replace('int','')
  text2 = text2.replace('float','')
  text2 = text2.replace('double','')
  text2 = text2.replace('*','')
  text2 = text2.replace(')','')
  text2 = text2.replace('(','')
  text2 = text2.replace('\n','')
  text2 = re.sub('\[[0-9]*\]','',text2)
  arg_list = []
  args = text2.split(',')
  for n in range(0,len(args)):
    arg_list.append(args[n].strip())
  return arg_list

def check_accs(name, arg_list, arg_typ, text):
  for n in range(0,len(arg_list)):
    if arg_typ[n] == 'ops_arg_dat':
      pos = 0
      while 1:
        match = re.search('\\b'+arg_list[n]+'\\b',text[pos:])
        if match == None:
          break
        pos = pos + match.start(0)
        if pos < 0:
          break
        pos = pos + len(arg_list[n])
        pos = pos + text[pos:].find('OPS_ACC_MD')
        pos2 = text[pos+10:].find('(')
        num = int(text[pos+10:pos+10+pos2])
        if num <> n:
          print 'Access mismatch in '+name+', arg '+str(n)+'('+arg_list[n]+') with OPS_ACC_MD'+str(num)
        pos = pos+10+pos2
        
def ops_gen_mpi(master, date, consts, kernels):

  global dims, stens
  global g_m, file_text, depth

  OPS_ID   = 1;  OPS_GBL   = 2;  OPS_MAP = 3;

  OPS_READ = 1;  OPS_WRITE = 2;  OPS_RW  = 3;
  OPS_INC  = 4;  OPS_MAX   = 5;  OPS_MIN = 6;

  accsstring = ['OPS_READ','OPS_WRITE','OPS_RW','OPS_INC','OPS_MAX','OPS_MIN' ]

  NDIM = 2 #the dimension of the application is hardcoded here .. need to get this dynamically

##########################################################################
#  create new kernel file
##########################################################################

  for nk in range (0,len(kernels)):
    arg_typ  = kernels[nk]['arg_type']
    name  = kernels[nk]['name']
    nargs = kernels[nk]['nargs']
    dim   = kernels[nk]['dim']
    dims  = kernels[nk]['dims']
    stens = kernels[nk]['stens']
    var   = kernels[nk]['var']
    accs  = kernels[nk]['accs']
    typs  = kernels[nk]['typs']
    NDIM = int(dim)
    #parse stencil to locate strided access
    stride = [1] * nargs * NDIM

    if NDIM == 2:
      for n in range (0, nargs):
        if str(stens[n]).find('STRID2D_X') > 0:
          stride[NDIM*n+1] = 0
        elif str(stens[n]).find('STRID2D_Y') > 0:
          stride[NDIM*n] = 0

    if NDIM == 3:
      for n in range (0, nargs):
        if str(stens[n]).find('STRID3D_X') > 0:
          stride[NDIM*n+1] = 0
          stride[NDIM*n+2] = 0
        elif str(stens[n]).find('STRID3D_Y') > 0:
          stride[NDIM*n] = 0
          stride[NDIM*n+2] = 0
        elif str(stens[n]).find('STRID3D_Z') > 0:
          stride[NDIM*n] = 0
          stride[NDIM*n+1] = 0


    reduction = 0
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_gbl' and accs[n] <> OPS_READ:
        reduction = 1

    arg_idx = 0
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_idx':
        arg_idx = 1

    g_m = 0;
    file_text = ''
    depth = 0
    n_per_line = 4

    i = name.find('kernel')
    name2 = name[0:i-1]
    #print name2

##########################################################################
#  generate MACROS
##########################################################################
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        code('int mdim'+str(n)+'_'+name+';')
    

    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        if NDIM==2:
          code('#define OPS_ACC_MD'+str(n)+'(d,x,y) ((x)*mdim'+str(n)+'_'+name+'+(d)+(xdim'+str(n)+'*(y)*mdim'+str(n)+'_'+name+'))')
        #if NDIM==3:
        #  code('#define OPS_ACC'+str(n)+'(x,y,z) (x+xdim'+str(n)+'_'+name+'*(y)+xdim'+str(n)+'_'+name+'*ydim'+str(n)+'_'+name+'*(z))')
    
##########################################################################
#  start with seq kernel function
##########################################################################

    code('')
    comm('user function')
    #code('#include "'+name2+'_kernel.h"')
    
    fid = open(name2+'_kernel.h', 'r')
    text = fid.read()
    fid.close()
    text = comment_remover(text)
    text = remove_trailing_w_space(text)

    i = text.find(name)
    if(i < 0):
      print "\n********"
      print "Error: cannot locate user kernel function: "+name+" - Aborting code generation"
      exit(2)

    print name
    i2 = i
    i = text[0:i].rfind('\n') #reverse find
    j = text[i:].find('{')
    k = para_parse(text, i+j, '{', '}')
    arg_list = parse_signature(text[i2+len(name):i+j])
    check_accs(name, arg_list, arg_typ, text[i+j:k])
    code(text[i:k+2])
    code('')
    code('')
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        code('#undef OPS_ACC_MD'+str(n))
    code('')
    code('')

##########################################################################
#  now host stub
##########################################################################

    comm(' host stub function')
    code('void ops_par_loop_'+name+'(char const *name, ops_block block, int dim, int* range,')
    text = ''
    for n in range (0, nargs):

      text = text +' ops_arg arg'+str(n)
      if nargs <> 1 and n != nargs-1:
        text = text +','
      else:
        text = text +') {'
      if n%n_per_line == 3 and n <> nargs-1:
         text = text +'\n'
    code(text);
    depth = 2

    code('');
    code('char *p_a['+str(nargs)+'];')
    code('int  offs['+str(nargs)+']['+dim+'];')

    #code('ops_printf("In loop \%s\\n","'+name+'");')

    text ='ops_arg args['+str(nargs)+'] = {'
    for n in range (0, nargs):
      text = text +' arg'+str(n)
      if nargs <> 1 and n != nargs-1:
        text = text +','
      else:
        text = text +'};\n\n'
      if n%n_per_line == 5 and n <> nargs-1:
        text = text +'\n                    '
    code(text);
    code('')
    code('#ifdef CHECKPOINTING')
    code('if (!ops_checkpointing_before(args,'+str(nargs)+',range,'+str(nk)+')) return;')
    code('#endif')
    code('')
    code('ops_timing_realloc('+str(nk)+',"'+name+'");')
    code('OPS_kernels['+str(nk)+'].count++;')
    code('')
    comm('compute locally allocated range for the sub-block')
    code('int start['+str(NDIM)+'];')
    code('int end['+str(NDIM)+'];')
    code('')

    code('#ifdef OPS_MPI')
    code('sub_block_list sb = OPS_sub_block_list[block->index];')
    code('if (!sb->owned) return;')
    FOR('n','0',str(NDIM))
    code('start[n] = sb->decomp_disp[n];end[n] = sb->decomp_disp[n]+sb->decomp_size[n];')
    IF('start[n] >= range[2*n]')
    code('start[n] = 0;')
    ENDIF()
    ELSE()
    code('start[n] = range[2*n] - start[n];')
    ENDIF()
    code('if (sb->id_m[n]==MPI_PROC_NULL && range[2*n] < 0) start[n] = range[2*n];')
    IF('end[n] >= range[2*n+1]')
    code('end[n] = range[2*n+1] - sb->decomp_disp[n];')
    ENDIF()
    ELSE()
    code('end[n] = sb->decomp_size[n];')
    ENDIF()
    code('if (sb->id_p[n]==MPI_PROC_NULL && (range[2*n+1] > sb->decomp_disp[n]+sb->decomp_size[n]))')
    code('  end[n] += (range[2*n+1]-sb->decomp_disp[n]-sb->decomp_size[n]);')
    ENDFOR()
    code('#else //OPS_MPI')
    FOR('n','0',str(NDIM))
    code('start[n] = range[2*n];end[n] = range[2*n+1];')
    ENDFOR()
    code('#endif //OPS_MPI')

    code('#ifdef OPS_DEBUG')
    code('ops_register_args(args, "'+name+'");')
    code('#endif')
    code('')

    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        code('offs['+str(n)+'][0] = args['+str(n)+'].stencil->stride[0]*1;  //unit step in x dimension')
        for d in range (1, NDIM):
          code('offs['+str(n)+']['+str(d)+'] = off'+str(NDIM)+'D('+str(d)+', &start[0],')
          if d == 1:
            code('    &end[0],args['+str(n)+'].dat->size, args['+str(n)+'].stencil->stride) - offs['+str(n)+']['+str(d-1)+'];')
          if d == 2:
            code('    &end[0],args['+str(n)+'].dat->size, args['+str(n)+'].stencil->stride) - offs['+str(n)+']['+str(d-1)+'] - offs['+str(n)+']['+str(d-2)+'];')
        code('')

    code('')
    if arg_idx:
      code('int arg_idx['+str(NDIM)+'];')
      code('#ifdef OPS_MPI')
      for n in range (0,NDIM):
        code('arg_idx['+str(n)+'] = sb->decomp_disp['+str(n)+']+start['+str(n)+'];')
      code('#else //OPS_MPI')
      for n in range (0,NDIM):
        code('arg_idx['+str(n)+'] = start['+str(n)+'];')
      code('#endif //OPS_MPI')


    code('')

    comm('Timing')
    code('double t1,t2,c1,c2;')
    code('ops_timers_core(&c2,&t2);')
    code('')

    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        for d in range (0, NDIM):
          code('int off'+str(n)+'_'+str(d)+' = offs['+str(n)+']['+str(d)+'];')
        code('int dat'+str(n)+' = args['+str(n)+'].dat->elem_size;')
        

    code('')
    comm('set up initial pointers and exchange halos if necessary')
    code('int d_m[OPS_MAX_DIM];')
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':

        #compute max halo depths using stencil
        #code('int max'+str(n)+'['+str(NDIM)+']; int min'+str(n)+'['+str(NDIM)+'];')
        #FOR('n','0',str(NDIM))
        #code('max'+str(n)+'[n] = 0;min'+str(n)+'[n] = 0;')
        #ENDFOR()
        #FOR('p','0','args['+str(n)+'].stencil->points')
        #FOR('n','0',str(NDIM))
        #code('max'+str(n)+'[n] = MAX(max'+str(n)+'[n],args['+str(n)+'].stencil->stencil['+str(NDIM)+'*p + n]);')# * ((range[2*n+1]-range[2*n]) == 1 ? 0 : 1);');
        #code('min'+str(n)+'[n] = MIN(min'+str(n)+'[n],args['+str(n)+'].stencil->stencil['+str(NDIM)+'*p + n]);')# * ((range[2*n+1]-range[2*n]) == 1 ? 0 : 1);');
        #ENDFOR()
        #ENDFOR()

        code('#ifdef OPS_MPI')
        code('for (int d = 0; d < dim; d++) d_m[d] = args['+str(n)+'].dat->d_m[d] + OPS_sub_dat_list[args['+str(n)+'].dat->index]->d_im[d];')
        code('#else //OPS_MPI')
        code('for (int d = 0; d < dim; d++) d_m[d] = args['+str(n)+'].dat->d_m[d];')
        code('#endif //OPS_MPI')
        code('int base'+str(n)+' = dat'+str(n)+' * 1 * ')
        code('  (start[0] * args['+str(n)+'].stencil->stride[0] - args['+str(n)+'].dat->base[0] - d_m[0]);')
        for d in range (1, NDIM):
          line = 'base'+str(n)+' = base'+str(n)+'+ dat'+str(n)+' *\n'
          for d2 in range (0,d):
            line = line + depth*' '+'  args['+str(n)+'].dat->size['+str(d2)+'] *\n'
          code(line[:-1])
          code('  (start['+str(d)+'] * args['+str(n)+'].stencil->stride['+str(d)+'] - args['+str(n)+'].dat->base['+str(d)+'] - d_m['+str(d)+']);')

        code('p_a['+str(n)+'] = (char *)args['+str(n)+'].data + base'+str(n)+';')

        #original address calculation via funcion call
        #code('p_a['+str(n)+'] = (char *)args['+str(n)+'].data')
        #code('+ address2('+str(NDIM)+', args['+str(n)+'].dat->elem_size, &start['+str(n)+'*'+str(NDIM)+'],')
        #code('args['+str(n)+'].dat->size, args['+str(n)+'].stencil->stride, args['+str(n)+'].dat->offset);')

      elif arg_typ[n] == 'ops_arg_gbl':
        if accs[n] == OPS_READ:
          code('p_a['+str(n)+'] = args['+str(n)+'].data;')
        else:
          code('#ifdef OPS_MPI')
          code('p_a['+str(n)+'] = ((ops_reduction)args['+str(n)+'].data)->data + ((ops_reduction)args['+str(n)+'].data)->size * block->index;')
          code('#else //OPS_MPI')
          code('p_a['+str(n)+'] = ((ops_reduction)args['+str(n)+'].data)->data;')
          code('#endif //OPS_MPI')
        code('')
      elif arg_typ[n] == 'ops_arg_idx':
        code('p_a['+str(n)+'] = (char *)arg_idx;')
        code('')
      #if arg_typ[n] == 'ops_arg_dat' and (accs[n] == OPS_READ or accs[n] == OPS_RW ):# or accs[n] == OPS_INC):
        #code('ops_exchange_halo2(&args['+str(n)+'],max'+str(n)+',min'+str(n)+');')
        #code('ops_exchange_halo3(&args['+str(n)+'],max'+str(n)+',min'+str(n)+',range);')
        #code('ops_exchange_halo(&args['+str(n)+'],2);')
      code('')
    code('')

    code('ops_H_D_exchanges_host(args, '+str(nargs)+');')
    code('ops_halo_exchanges(args,'+str(nargs)+',range);')
    code('ops_H_D_exchanges_host(args, '+str(nargs)+');')
    code('')
    code('ops_timers_core(&c1,&t1);')
    code('OPS_kernels['+str(nk)+'].mpi_time += t1-t2;')
    code('')



    comm("initialize global variable with the dimension of dats")
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        code('mdim'+str(n)+'_'+name+' = args['+str(n)+'].dat->dim;')
        code('xdim'+str(n)+' = args['+str(n)+'].dat->size[0]*args['+str(n)+'].dat->dim;')
        if NDIM==3:
          code('ydim'+str(n)+' = args['+str(n)+'].dat->size[1];')
    code('')

    code('int n_x;')

    if NDIM==3:
      FOR('n_z','start[2]','end[2]')

    FOR('n_y','start[1]','end[1]')
    #FOR('n_x','start[0]','start[0]+(end[0]-start[0])/SIMD_VEC')
    #FOR('n_x','start[0]','start[0]+(end[0]-start[0])/SIMD_VEC')
    #code('for( n_x=0; n_x<ROUND_DOWN((end[0]-start[0]),SIMD_VEC); n_x+=SIMD_VEC ) {')
    code('#pragma novector')
    code('for( n_x=start[0]; n_x<start[0]+((end[0]-start[0])/SIMD_VEC)*SIMD_VEC; n_x+=SIMD_VEC ) {')
    depth = depth+2

    comm('call kernel function, passing in pointers to data -vectorised')
    if reduction == 0 and arg_idx == 0:
      code('#pragma simd')
    FOR('i','0','SIMD_VEC')
    text = name+'( '
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        text = text +' ('+typs[n]+' *)p_a['+str(n)+']+ i*'+str(stride[NDIM*n])+'*mdim'+str(n)+'_'+name
      else:
        text = text +' ('+typs[n]+' *)p_a['+str(n)+']'
      if nargs <> 1 and n != nargs-1:
        text = text + ','
      else:
        text = text +' );\n'
      if n%n_per_line == 2 and n <> nargs-1:
        text = text +'\n          '
    code(text);
    if arg_idx:
      code('arg_idx[0]++;')
    ENDFOR()
    code('')


    comm('shift pointers to data x direction')
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
          code('p_a['+str(n)+']= p_a['+str(n)+'] + (dat'+str(n)+' * off'+str(n)+'_0)*SIMD_VEC;')

    ENDFOR()
    code('')


    FOR('n_x','start[0]+((end[0]-start[0])/SIMD_VEC)*SIMD_VEC','end[0]')
    #code('for(;n_x<(end[0]-start[0]);n_x++) {')
    #depth = depth+2
    comm('call kernel function, passing in pointers to data - remainder')
    text = name+'( '
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        text = text +' ('+typs[n]+' *)p_a['+str(n)+']'
      else:
        text = text +' ('+typs[n]+' *)p_a['+str(n)+']'
      if nargs <> 1 and n != nargs-1:
        text = text + ','
      else:
        text = text +' );\n'
      if n%n_per_line == 2 and n <> nargs-1:
        text = text +'\n          '
    code(text);

    code('')


    comm('shift pointers to data x direction')
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
          code('p_a['+str(n)+']= p_a['+str(n)+'] + (dat'+str(n)+' * off'+str(n)+'_0);')

    if arg_idx:
      code('arg_idx[0]++;')
    ENDFOR()
    code('')


    comm('shift pointers to data y direction')
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
          #code('p_a['+str(n)+']= p_a['+str(n)+'] + (dat'+str(n)+' * (off'+str(n)+'_1) - '+str(stride[NDIM*n])+');')
          code('p_a['+str(n)+']= p_a['+str(n)+'] + (dat'+str(n)+' * off'+str(n)+'_1);')
    if arg_idx:
      code('#ifdef OPS_MPI')
      for n in range (0,1):
        code('arg_idx['+str(n)+'] = sb->decomp_disp['+str(n)+']+start['+str(n)+'];')
      code('#else //OPS_MPI')
      for n in range (0,1):
        code('arg_idx['+str(n)+'] = start['+str(n)+'];')
      code('#endif //OPS_MPI')
      code('arg_idx[1]++;')
    ENDFOR()

    if NDIM==3:
      comm('shift pointers to data z direction')
      for n in range (0, nargs):
        if arg_typ[n] == 'ops_arg_dat':
            #code('p_a['+str(n)+']= p_a['+str(n)+'] + (dat'+str(n)+' * (off'+str(n)+'_2) - '+str(stride[NDIM*n])+');')
            code('p_a['+str(n)+']= p_a['+str(n)+'] + (dat'+str(n)+' * off'+str(n)+'_2);')

      if arg_idx:
        code('#ifdef OPS_MPI')
        for n in range (0,2):
          code('arg_idx['+str(n)+'] = sb->decomp_disp['+str(n)+']+start['+str(n)+'];')
        code('#else //OPS_MPI')
        for n in range (0,2):
          code('arg_idx['+str(n)+'] = start['+str(n)+'];')
        code('#endif //OPS_MPI')
        code('arg_idx[2]++;')
      ENDFOR()

    code('ops_timers_core(&c2,&t2);')
    code('OPS_kernels['+str(nk)+'].time += t2-t1;')

    # if reduction == 1 :
    #   for n in range (0, nargs):
    #     if arg_typ[n] == 'ops_arg_gbl' and accs[n] != OPS_READ:
    #       #code('ops_mpi_reduce(&arg'+str(n)+',('+typs[n]+' *)p_a['+str(n)+']);')

    #   code('ops_timers_core(&c1,&t1);')
    #   code('OPS_kernels['+str(nk)+'].mpi_time += t1-t2;')

    code('ops_set_dirtybit_host(args, '+str(nargs)+');')
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat' and (accs[n] == OPS_WRITE or accs[n] == OPS_RW or accs[n] == OPS_INC):
        #code('ops_set_halo_dirtybit(&args['+str(n)+']);')
        code('ops_set_halo_dirtybit3(&args['+str(n)+'],range);')

    # code('')
    # code('#ifdef OPS_DEBUG')
    # for n in range (0,nargs):
    #   if arg_typ[n] == 'ops_arg_dat' and accs[n] <> OPS_READ:
    #     code('ops_dump3(arg'+str(n)+'.dat,"'+name+'");')
    # code('#endif')
    code('')
    comm('Update kernel record')
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        code('OPS_kernels['+str(nk)+'].transfer += ops_compute_transfer(dim, range, &arg'+str(n)+');')
    depth = depth - 2
    code('}')

##########################################################################
#  output individual kernel file
##########################################################################
    if not os.path.exists('./MPI'):
      os.makedirs('./MPI')
    fid = open('./MPI/'+name+'_seq_kernel.cpp','w')
    date = datetime.datetime.now()
    fid.write('//\n// auto-generated by ops.py\n//\n')
    fid.write(file_text)
    fid.close()

# end of main kernel call loop

##########################################################################
#  output one master kernel file
##########################################################################
  depth = 0
  file_text =''
  comm('header')
  if NDIM==3:
    code('#define OPS_3D')
  code('#include "ops_lib_cpp.h"')
  code('#ifdef OPS_MPI')
  code('#include "ops_mpi_core.h"')
  code('#endif')
  if os.path.exists('./user_types.h'):
    code('#include "user_types.h"')
  code('')

  comm(' global constants')
  for nc in range (0,len(consts)):
    if consts[nc]['dim'].isdigit() and int(consts[nc]['dim'])==1:
      code('extern '+consts[nc]['type']+' '+(str(consts[nc]['name']).replace('"','')).strip()+';')
#      code('#pragma acc declare create('+(str(consts[nc]['name']).replace('"','')).strip()+')')
    else:
      if consts[nc]['dim'].isdigit() and consts[nc]['dim'] > 0:
        num = str(consts[nc]['dim'])
        code('extern '+consts[nc]['type']+' '+(str(consts[nc]['name']).replace('"','')).strip()+'['+num+'];')
#        code('#pragma acc declare create('+(str(consts[nc]['name']).replace('"','')).strip()+')')
      else:
        code('extern '+consts[nc]['type']+' *'+(str(consts[nc]['name']).replace('"','')).strip()+';')
#        code('#pragma acc declare create('+(str(consts[nc]['name']).replace('"','')).strip()+')')

  #constants for macros -- now included in teh backend so no need to generate here
  #for i in range(0,20):
  #  code('int xdim'+str(i)+';')
  #code('')

  comm('user kernel files')

  kernel_name_list = []

  for nk in range(0,len(kernels)):
    if kernels[nk]['name'] not in kernel_name_list :
      code('#include "'+kernels[nk]['name']+'_seq_kernel.cpp"')
      kernel_name_list.append(kernels[nk]['name'])

  master = master.split('.')[0]
  fid = open('./MPI/'+master.split('.')[0]+'_seq_kernels.cpp','w')
  fid.write('//\n// auto-generated by ops.py//\n\n')
  fid.write(file_text)
  fid.close()
