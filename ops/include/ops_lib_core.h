/*
* Open source copyright declaration based on BSD open source template:
* http://www.opensource.org/licenses/bsd-license.php
*
* This file is part of the OPS distribution.
*
* Copyright (c) 2013, Mike Giles and others. Please see the AUTHORS file in
* the main source directory for a full list of copyright holders.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * The name of Mike Giles may not be used to endorse or promote products
* derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Mike Giles ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Mike Giles BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/** @brief ops core library function declarations
  * @author Gihan Mudalige
  * @details function declarations headder file for the core library functions
  * utilized by all OPS backends
  */

#ifndef __OPS_LIB_CORE_H
#define __OPS_LIB_CORE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <stdarg.h>
#include <sys/queue.h> //contains double linked list implementation
#include <stdbool.h>

#include "ops_util.h"


/*
* enum list for ops_par_loop
*/

#define OPS_READ 0
#define OPS_WRITE 1
#define OPS_RW 2
#define OPS_INC 3
#define OPS_MIN 4
#define OPS_MAX 5

#define OPS_ARG_GBL 0
#define OPS_ARG_DAT 1
#define OPS_ARG_IDX 2

#ifdef MAX_DEPTH
#undef MAX_DEPTH
#endif
#define MAX_DEPTH 5

#define OPS_MAX_DIM 5

/*
 * * zero constants
 * */

#define ZERO_double 0.0;
#define INFINITY_double INFINITY;
#define ZERO_float 0.0f;
#define INFINITY_float INFINITY;
#define ZERO_int 0;
#define INFINITY_int INFINITY;
#define ZERO_uint 0;
#define INFINITY_uint INFINITY;
#define ZERO_ll 0;
#define INFINITY_ll INFINITY;
#define ZERO_ull 0;
#define INFINITY_ull INFINITY;
#define ZERO_bool 0;




#ifdef __cplusplus
extern "C" {
#endif

/*
* essential typedefs
*/

typedef unsigned int uint;
typedef long long ll;
typedef unsigned long long ull;

typedef int ops_access; //holds OP_READ, OP_WRITE, OP_RW, OP_INC, OP_MIN, OP_MAX
typedef int ops_arg_type; // holds OP_ARG_GBL, OP_ARG_DAT


/*
* structures
*/

typedef struct
{
  int         index;  /* index */
  int         dims;   /* dimension of block, 2D,3D .. etc*/
  int         *size;  /* size of block in each dimension */
  char const  *name;  /* name of block */
} ops_block_core;

typedef ops_block_core * ops_block;

typedef struct
{
  int         index;       /* index */
  ops_block   block;       /* block on which data is defined */
  int         dim;        /* number of elements per grid point*/
  int         size;        /* number of bytes per grid point*/
  int         *block_size; /* size of the array in each block dimension -- including halo*/
  int         *offset;     /* depth from the start of each dimention*/
  int         *tail;       /* depth from the end of each dimention*/
  char        *data;       /* data on host */
  char        *data_d;     /* data on device */
  char const  *name;       /* name of dataset */
  char const  *type;       /* datatype */
  int         dirtybit;    /* flag to indicate MPI halo exchange is needed*/
  int         dirty_hd;    /* flag to indicate dirty status on host and device */
  int*        dirty_dir_send;   /* flag to indicate MPI halo exchange in a direction is needed*/
  int*        dirty_dir_recv;   /* flag to indicate MPI halo exchange in a direction is needed*/
  int         user_managed;/* indicates whether the user is managing memory */
  int         e_dat;    /* is this an edge dat?*/
} ops_dat_core;

typedef ops_dat_core * ops_dat;

//struct definition for a double linked list entry to hold an ops_dat
struct ops_dat_entry_core{
  ops_dat dat;
  TAILQ_ENTRY(ops_dat_entry_core) entries; /*holds pointers to next and
                                             previous entries in the list*/
};

typedef struct ops_dat_entry_core ops_dat_entry;

typedef TAILQ_HEAD(, ops_dat_entry_core) Double_linked_list;

typedef struct
{
        int         index;     /* index */
        int         dims;      /* dimensionality of the stencil */
        int         points;    /* number of stencil elements */
        char const  *name;     /* name of pointer */
        int         *stencil;  /* elements in the stencil */
        int         *stride;   /* stride of the stencil */
} ops_stencil_core;

typedef ops_stencil_core * ops_stencil;

typedef struct
{
  ops_dat     dat;     /* dataset */
  ops_stencil stencil; /* the stencil */
  int         dim;     /* dimension of data */
  char        *data;   /* data on host */
  char        *data_d; /* data on device (for CUDA)*/
  ops_access   acc;    /* access type */
  ops_arg_type argtype;/* arg type */
  int         opt;     /*falg to indicate whether this is an optional arg, 0 - optional, 1 - not optional*/
} ops_arg;

typedef struct
{
  char       *name;     /* name of kernel function */
  int         count;    /* number of times called */
  double       time;     /* total execution time */
  float       transfer; /* bytes of data transfer (used) */
  double       mpi_time; /* time spent in MPI calls */
//  double       mpi_gather;
//  double       mpi_scatter;
//  double       mpi_sendrecv;
//  double       mpi_reduct;
} ops_kernel;


typedef struct
{
  int         count;       /* number of blocks */
  int         blocklength; /*size of blocks */
  int         stride;      /*stride between blocks */
} ops_halo;



//
//Struct for holding the decomposition details of a block on an MPI process
//
typedef struct {
  // the decomposition is for this block
  ops_block block;
  //number of dimensions;
  int ndim;
  // my MPI rank in each dimension (in cart cords)
  int* coords;
  // previous neighbor in each dimension (in cart cords)
  int* id_m;
  // next neighbor in each dimension (in cart cords)
  int* id_p;
  // the size of the local sub-block in each dimension
  int* sizes;
  // the displacement from the start of the block in each dimension
  int* disps;
  // the global index of the starting element of the local sub-block in each dimension
  int* istart;
  // the global index of the starting element of the local sub-block
  int* iend;

} sub_block;

typedef sub_block * sub_block_list;


typedef struct ops_kernel_descriptor
{
//  char        *name;     /* name of kernel */
  ops_arg     *args;     /* list of arguments to pass in */
  int         nargs;     /* number of arguments */
  int           dim;     /* number of dimensions */
  int      range[6];     /* execution range */
  ops_block   block;     /* block to execute on */
  void (*function)(struct ops_kernel_descriptor *desc); /* Function pointer to a wrapper to be called */
} ops_kernel_descriptor;


/*
* min / max definitions
*/

#ifndef MIN
#define MIN(a,b) ((a<b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a>b) ? (a) : (b))
#endif
#ifndef SIGN
#define SIGN(a,b) ((b<0.0) ? (a*(-1)) : (a))
#endif

/*
 * alignment macro based on example on page 50 of CUDA Programming Guide version 3.0
 * rounds up to nearest multiple of 16 bytes
 */

#define ROUND_UP(bytes) (((bytes) + 15) & ~15)
#define ROUND_UP_64(bytes) (((bytes) + 63) & ~63)


/*******************************************************************************
* Global constants
*******************************************************************************/

extern int OPS_hybrid_gpu;
extern int OPS_kern_max, OPS_kern_curr;
extern ops_kernel * OPS_kernels;

extern int ops_current_kernel;

extern int OPS_diags;

extern int OPS_block_index, OPS_block_max,
           OPS_dat_index, OPS_dat_max;

extern ops_block * OPS_block_list;
extern Double_linked_list OPS_dat_list; //Head of the double linked list
extern ops_arg *OPS_curr_args;

extern sub_block_list *OPS_sub_block_list;

/*******************************************************************************
* Core lib function prototypes
*******************************************************************************/


void ops_init( int argc, char **argv, int diags_level );
void ops_exit();
void ops_init_core( int argc, char **argv, int diags_level );
void ops_exit_core( void );

/* ops_block */
ops_block ops_decl_block(int dims, int *size, char *name);

/* ops_dat */
ops_dat ops_decl_dat_core( ops_block block, int data_size,
                      int *block_size, int* offset, int* tail, char *data, int type_size,
                      char const * type,
                      char const * name );

ops_dat ops_decl_dat_temp_core( ops_block block, int data_size,
                      int *block_size, int* offset,  int* tail, char * data, int type_size,
                      char const * type, char const * name );

ops_dat ops_decl_dat_char(ops_block, int, int*, int*, int*, char *, int, char const*, char const* );
ops_dat ops_decl_dat_mpi_char(ops_block block, int size, int *dat_size, int* offset, int* tail,
                      char* data, int type_size, char const * type, char const * name );


/* ops_stencil */
ops_stencil ops_decl_stencil( int dims, int points, int *stencil, char const * name);
ops_stencil ops_decl_strided_stencil( int dims, int points, int *sten, int *stride, char const * name);

/* ops_arg */
ops_arg ops_arg_dat( ops_dat dat, ops_stencil stencil, char const * type, ops_access acc );
ops_arg ops_arg_dat_opt( ops_dat dat, ops_stencil stencil, char const * type, ops_access acc, int flag );
ops_arg ops_arg_idx( );
ops_arg ops_arg_gbl_char( char * data, int dim, int size, ops_access acc );
ops_arg ops_arg_dat_core( ops_dat dat, ops_stencil stencil, ops_access acc );
ops_arg ops_arg_gbl_core( char * data, int dim, int size, ops_access acc );
ops_arg ops_arg_idx_core( );

/* constants */
void ops_decl_const_char( int, char const *, int, char *, char const* );
void ops_decl_const_core( int dim, char const * type, int typeSize, char * data, char const * name );

/* misc functions */
void ops_printf(const char* format, ...);
void ops_fprintf(FILE *stream, const char *format, ...);

void ops_diagnostic_output( );
void ops_timing_output();

int ops_is_root();

void ops_partition(int dims, int* size, char* routine);

void ops_timers( double *cpu, double *et );
void ops_print_dat_to_txtfile(ops_dat dat, const char *file_name);
void ops_print_dat_to_txtfile_core(ops_dat dat, const char* file_name);
void ops_dump3(ops_dat dat, const char *name);

void ops_timing_realloc ( int, const char * );
void ops_timers_core( double *cpu, double *et );

/*******************************************************************************
* Internals
*******************************************************************************/

float ops_compute_transfer(int dims, int *range, ops_arg *arg);
void ops_register_args(ops_arg *args, const char *name);
int ops_stencil_check_2d(int arg_idx, int idx0, int idx1, int dim0, int dim1);
int ops_stencil_check_3d(int arg_idx, int idx0, int idx1, int idx2, int dim0, int dim1);

/* keeping data up-to-date */
void ops_set_dirtybit_host(ops_arg *args, int nargs); //data updated on host .. i.e. dirty on host
void ops_set_halo_dirtybit(ops_arg *arg);
void ops_set_halo_dirtybit3(ops_arg *arg, int *iter_range);
void ops_halo_exchanges(ops_arg* args, int nargs, int *range);
void ops_exchange_halo(ops_arg* arg, int d /*depth*/);
void ops_exchange_halo2(ops_arg* arg, int* d_pos, int* d_neg /*depth*/);
void ops_exchange_halo3(ops_arg* arg, int* d_pos, int* d_neg /*depth*/, int *iter_range);
void ops_set_dirtybit_cuda(ops_arg *args, int nargs);
void ops_H_D_exchanges(ops_arg *args, int nargs);
void ops_H_D_exchanges_cuda(ops_arg *args, int nargs);

/* reductions */
void ops_mpi_reduce_float(ops_arg *args, float* data);
void ops_mpi_reduce_double(ops_arg *args, double* data);
void ops_mpi_reduce_int(ops_arg *args, int* data);

void ops_compute_moment(double t, double *first, double *second);

/* lazy execution */
void ops_enqueue_kernel(ops_kernel_descriptor *desc);
void ops_execute();

#ifdef __cplusplus
}
#endif

#endif /* __OP_LIB_CORE_H */
