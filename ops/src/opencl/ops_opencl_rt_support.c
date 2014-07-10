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

/** @brief ops opencl specific runtime support functions
  * @author Gihan Mudalige and Istvan Reguly (adapting OP2 OpenCL backend by Endre Lazslo)
  * @details Implements opencl backend runtime support functions
  */


//
// header files
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#include <CL/cl_ext.h>
#endif

//#include <math_constants.h>

#include <ops_lib_core.h>
#include <ops_opencl_rt_support.h>


// Small re-declaration to avoid using struct in the C version.
// This is due to the different way in which C and C++ see structs

typedef struct cudaDeviceProp cudaDeviceProp_t;

int OPS_consts_bytes = 0,
    OPS_reduct_bytes = 0;

char * OPS_consts_h,
     * OPS_consts_d,
     * OPS_reduct_h,
     * OPS_reduct_d;

int OPS_gbl_changed = 1;
char *OPS_gbl_prev = NULL;

ops_opencl_core OPS_opencl_core;
int OPS_cl_device = 0; //select CPU by default

//
// Get return (error) messages from OpenCL run-time
//
char *clGetErrorString(cl_int err) {
  switch (err) {
    case CL_SUCCESS: return (char *) "Success!";
    case CL_DEVICE_NOT_FOUND: return (char *) "Device not found.";
    case CL_DEVICE_NOT_AVAILABLE: return (char *) "Device not available";
    case CL_COMPILER_NOT_AVAILABLE: return (char *) "Compiler not available";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE: return (char *) "Memory object allocation failure";
    case CL_OUT_OF_RESOURCES: return (char *) "Out of resources";
    case CL_OUT_OF_HOST_MEMORY: return (char *) "Out of host memory";
    case CL_PROFILING_INFO_NOT_AVAILABLE: return (char *) "Profiling information not available";
    case CL_MEM_COPY_OVERLAP: return (char *) "Memory copy overlap";
    case CL_IMAGE_FORMAT_MISMATCH: return (char *) "Image format mismatch";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED: return (char *) "Image format not supported";
    case CL_BUILD_PROGRAM_FAILURE: return (char *) "Program build failure";
    case CL_MAP_FAILURE: return (char *) "Map failure";
    case CL_MISALIGNED_SUB_BUFFER_OFFSET: return (char*) "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return (char*) "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case CL_INVALID_VALUE: return (char *) "Invalid value";
    case CL_INVALID_DEVICE_TYPE: return (char *) "Invalid device type";
    case CL_INVALID_PLATFORM: return (char *) "Invalid platform";
    case CL_INVALID_DEVICE: return (char *) "Invalid device";
    case CL_INVALID_CONTEXT: return (char *) "Invalid context";
    case CL_INVALID_QUEUE_PROPERTIES: return (char *) "Invalid queue properties";
    case CL_INVALID_COMMAND_QUEUE: return (char *) "Invalid command queue";
    case CL_INVALID_HOST_PTR: return (char *) "Invalid host pointer";
    case CL_INVALID_MEM_OBJECT: return (char *) "Invalid memory object";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return (char *) "Invalid image format descriptor";
    case CL_INVALID_IMAGE_SIZE: return (char *) "Invalid image size";
    case CL_INVALID_SAMPLER: return (char *) "Invalid sampler";
    case CL_INVALID_BINARY: return (char *) "Invalid binary";
    case CL_INVALID_BUILD_OPTIONS: return (char *) "Invalid build options";
    case CL_INVALID_PROGRAM: return (char *) "Invalid program";
    case CL_INVALID_PROGRAM_EXECUTABLE: return (char *) "Invalid program executable";
    case CL_INVALID_KERNEL_NAME: return (char *) "Invalid kernel name";
    case CL_INVALID_KERNEL_DEFINITION: return (char *) "Invalid kernel definition";
    case CL_INVALID_KERNEL: return (char *) "Invalid kernel";
    case CL_INVALID_ARG_INDEX: return (char *) "Invalid argument index";
    case CL_INVALID_ARG_VALUE: return (char *) "Invalid argument value";
    case CL_INVALID_ARG_SIZE: return (char *) "Invalid argument size";
    case CL_INVALID_KERNEL_ARGS: return (char *) "Invalid kernel arguments";
    case CL_INVALID_WORK_DIMENSION: return (char *) "Invalid work dimension";
    case CL_INVALID_WORK_GROUP_SIZE: return (char *) "Invalid work group size";
    case CL_INVALID_WORK_ITEM_SIZE: return (char *) "Invalid work item size";
    case CL_INVALID_GLOBAL_OFFSET: return (char *) "Invalid global offset";
    case CL_INVALID_EVENT_WAIT_LIST: return (char *) "Invalid event wait list";
    case CL_INVALID_EVENT: return (char *) "Invalid event";
    case CL_INVALID_OPERATION: return (char *) "Invalid operation";
    case CL_INVALID_GL_OBJECT: return (char *) "Invalid OpenGL object";
    case CL_INVALID_BUFFER_SIZE: return (char *) "Invalid buffer size";
    case CL_INVALID_MIP_LEVEL: return (char *) "Invalid mip-map level";
    case CL_INVALID_GLOBAL_WORK_SIZE: return (char *) "Invalid global work size";
    case CL_INVALID_PROPERTY: return (char *) "Invalid property";
    default: return (char *) "Unknown";
  }
}


//
// OpenCL utility functions
//

void __clSafeCall(cl_int ret, const char * file, const int line) {
  if ( CL_SUCCESS != ret ) {
    ops_fprintf ( stderr, "%s(%i) : clSafeCall() Runtime API error : %s.\n",
        file, line, clGetErrorString ( ret ) );
    exit ( -1 );
  }
}


void pfn_notify(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
  fprintf(stderr, "OpenCL Error (via pfn_notify) errinfo : %s\n", errinfo);
  fprintf(stderr, "OpenCL Error (via pfn_notify) private info: %s\n", (const char*)private_info);
}


void openclDeviceInit( int argc, char ** argv )
{
  (void)argc;
  (void)argv;

  cl_int ret;
  char buffer[10240];
  cl_bool available = false;
  float *test_h1 = (float*) malloc(sizeof(float));
  float *test_h2 = (float*) malloc(sizeof(float));
  cl_mem test_d = NULL;

  // Get platform and device information
  OPS_opencl_core.platform_id = NULL;
  OPS_opencl_core.device_id = NULL;
  OPS_opencl_core.constant = NULL;

  clSafeCall( clGetPlatformIDs(0, NULL, &OPS_opencl_core.n_platforms) );

  if(OPS_diags >0)
    ops_printf("num_platforms = %i \n",(int) OPS_opencl_core.n_platforms);

  OPS_opencl_core.platform_id = (cl_platform_id*) malloc( OPS_opencl_core.n_platforms*sizeof(cl_uint) );
  clSafeCall( clGetPlatformIDs( OPS_opencl_core.n_platforms, OPS_opencl_core.platform_id, NULL) );

  int device_type = OPS_cl_device;
  switch(device_type) {
    case 0://CPU:
      ret = clGetDeviceIDs(OPS_opencl_core.platform_id[0], CL_DEVICE_TYPE_CPU, 1,
        &OPS_opencl_core.device_id, &OPS_opencl_core.n_devices);
      break;
    case 1://GPU or PHI:
      ret = clGetDeviceIDs(OPS_opencl_core.platform_id[1], CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR, 1,
        &OPS_opencl_core.device_id, &OPS_opencl_core.n_devices);
      break;
  }

  if(OPS_diags >5)
    ops_printf("ret clGetDeviceIDs(.,%d,...) = %d\n", device_type,ret);

  if(OPS_opencl_core.n_platforms == 0 && OPS_opencl_core.n_devices == 0) {
    ops_printf("No OpenCL platform or device is available! Exiting.\n");
    exit(-1);
  }
  ops_printf("\nNo. of devices on platform = %d\n", OPS_opencl_core.n_devices);

  ops_printf("\nChosen device: \n");
  clSafeCall( clGetDeviceInfo(OPS_opencl_core.device_id, CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL) );
  ops_printf("\nCL_DEVICE_VENDOR = %s \n", buffer);
  clSafeCall( clGetDeviceInfo(OPS_opencl_core.device_id, CL_DEVICE_NAME, sizeof(buffer), buffer, NULL) );
  ops_printf("\nCL_DEVICE_NAME = %s \n", buffer);
  clSafeCall( clGetDeviceInfo(OPS_opencl_core.device_id, CL_DEVICE_AVAILABLE, sizeof(cl_bool), &available, NULL) );
  ops_printf("\nCL_DEVICE_AVAILABLE = %s \n", available ? "true" : "false");

  // Create an OpenCL context
  OPS_opencl_core.context = clCreateContext( NULL, 1, &OPS_opencl_core.device_id, &pfn_notify, NULL, &ret);
  clSafeCall( ret );

  // Create a command queue
  OPS_opencl_core.command_queue = clCreateCommandQueue(OPS_opencl_core.context, OPS_opencl_core.device_id, CL_QUEUE_PROFILING_ENABLE, &ret);
  clSafeCall( ret );

  // Make a read/write test
  test_h1[0] = 1986;
  test_d = clCreateBuffer(OPS_opencl_core.context, CL_MEM_READ_WRITE, sizeof(float), NULL, &ret);
  clSafeCall( ret );
  clSafeCall( clEnqueueWriteBuffer(OPS_opencl_core.command_queue, test_d, CL_TRUE, 0, sizeof(float), (void*)test_h1, 0, NULL, NULL) );
  clSafeCall( clEnqueueReadBuffer(OPS_opencl_core.command_queue, test_d, CL_TRUE, 0, sizeof(float), (void*)test_h2, 0, NULL, NULL) );
  if(test_h1[0] != test_h2[0]) {
    printf("Error during buffer read/write test! Exiting \n");
    exit(-1);
  }
  clSafeCall( clReleaseMemObject(test_d) );
  //clSafeCall( clFlush(OPS_opencl_core.command_queue) );
  clSafeCall( clFinish(OPS_opencl_core.command_queue) );

  // Number of constants in constant array
  OPS_opencl_core.n_constants = 0;
}

void ops_cpHostToDevice ( void ** data_d, void ** data_h, int size )
{
  //printf("Copying data from host to device\n");
  cl_int ret = 0;
  *data_d = (cl_mem) clCreateBuffer(OPS_opencl_core.context, CL_MEM_READ_WRITE, size, NULL, &ret);
  clSafeCall( ret );
  clSafeCall( clEnqueueWriteBuffer(OPS_opencl_core.command_queue, (cl_mem) *data_d, CL_TRUE, 0, size, *data_h, 0, NULL, NULL) );
  //clSafeCall( clFlush(OPS_opencl_core.command_queue) );
  clSafeCall( clFinish(OPS_opencl_core.command_queue) );
}


void ops_download_dat(ops_dat dat) {

  //if (!OPS_hybrid_gpu) return;
  int bytes = dat->elem_size;
  for (int i=0; i<dat->block->dims; i++) bytes = bytes * dat->size[i];

  //printf("downloading to host from device %d bytes\n",bytes);
  clSafeCall( clEnqueueReadBuffer(OPS_opencl_core.command_queue, (cl_mem) dat->data_d, CL_TRUE, 0, bytes, dat->data, 0, NULL, NULL) );
  //clSafeCall( clFlush(OPS_opencl_core.command_queue) );
  clSafeCall( clFinish(OPS_opencl_core.command_queue) );

}

void ops_upload_dat(ops_dat dat) {

  //if (!OPS_hybrid_gpu) return;
  int bytes = dat->elem_size;
  for (int i=0; i<dat->block->dims; i++) bytes = bytes * dat->size[i];
  //printf("uploading to device from host %d bytes\n",bytes);

  clSafeCall( clEnqueueWriteBuffer(OPS_opencl_core.command_queue, (cl_mem) dat->data_d, CL_TRUE, 0, bytes, dat->data, 0, NULL, NULL) );
  //clSafeCall( clFlush(OPS_opencl_core.command_queue) );
  clSafeCall( clFinish(OPS_opencl_core.command_queue) );


}

void ops_H_D_exchanges_host(ops_arg *args, int nargs)
{
  //printf("in ops_H_D_exchanges\n");
  for (int n=0; n<nargs; n++)
    if(args[n].argtype == OPS_ARG_DAT && args[n].dat->dirty_hd == 2) {
      ops_download_dat(args[n].dat);
      //printf("halo exchanges on host\n");
      args[n].dat->dirty_hd = 0;
    }
}

void ops_H_D_exchanges_device(ops_arg *args, int nargs)
{
  for (int n=0; n<nargs; n++)
    if(args[n].argtype == OPS_ARG_DAT && args[n].dat->dirty_hd == 1) {
      ops_upload_dat(args[n].dat);
      args[n].dat->dirty_hd = 0;
    }
}

void ops_set_dirtybit_host(ops_arg *args, int nargs)
{
  //printf("in ops_set_dirtybit\n");
  for (int n=0; n<nargs; n++) {
    if((args[n].argtype == OPS_ARG_DAT) &&
       (args[n].acc == OPS_INC || args[n].acc == OPS_WRITE || args[n].acc == OPS_RW) ) {
      //printf("setting dirty bit on host\n");
      args[n].dat->dirty_hd = 1;
    }
  }
}


void ops_set_dirtybit_device(ops_arg *args, int nargs)
{
  for (int n=0; n<nargs; n++) {
    if((args[n].argtype == OPS_ARG_DAT) &&
       (args[n].acc == OPS_INC || args[n].acc == OPS_WRITE || args[n].acc == OPS_RW) ) {
      args[n].dat->dirty_hd = 2;
    }
  }
}

//
// routine to fetch data from GPU to CPU (with transposing SoA to AoS if needed)
//

void ops_opencl_get_data( ops_dat dat )
{
  //if (!OPS_hybrid_gpu) return;
  if (dat->dirty_hd == 2) dat->dirty_hd = 0;
  else return;
  ops_download_dat(dat);

}


//
// routines to resize constant/reduct arrays, if necessary
//

void reallocConstArrays ( int consts_bytes )
{
  cl_int ret;
  if ( consts_bytes > OPS_consts_bytes ) {
    if ( OPS_consts_bytes > 0 ) {
      free ( OPS_consts_h );
      clSafeCall( clReleaseMemObject((cl_mem)OPS_consts_d) );
    }
    OPS_consts_bytes = 4 * consts_bytes; // 4 is arbitrary, more than needed
    OPS_consts_h = ( char * ) malloc ( OPS_consts_bytes );
    OPS_consts_d = (char*) clCreateBuffer(OPS_opencl_core.context, CL_MEM_READ_WRITE, OPS_consts_bytes, NULL, &ret);
    clSafeCall( ret );
  }
}


void reallocReductArrays ( int reduct_bytes )
{
  cl_int ret;
  if ( reduct_bytes > OPS_reduct_bytes ) {
    if ( OPS_reduct_bytes > 0 ) {
      free ( OPS_reduct_h );
      clSafeCall( clReleaseMemObject((cl_mem)OPS_reduct_d) );
    }
    OPS_reduct_bytes = 4 * reduct_bytes; // 4 is arbitrary, more than needed
    OPS_reduct_h = ( char * ) malloc ( OPS_reduct_bytes );
    OPS_reduct_d = (char*) clCreateBuffer(OPS_opencl_core.context, CL_MEM_READ_WRITE, OPS_reduct_bytes, NULL, &ret);
    clSafeCall( ret );
  }
}

//
// routines to move constant/reduct arrays
//



void mvConstArraysToDevice ( int consts_bytes )
{
  OPS_gbl_changed=0;
  if(OPS_gbl_prev != NULL)
    for(int i = 0; i<consts_bytes; i++) { 
      if (OPS_consts_h[i] != OPS_gbl_prev[i]) OPS_gbl_changed=1;
    }
  else 
  {  
    OPS_gbl_changed = 1;
    OPS_gbl_prev = (char *)malloc(consts_bytes);
  }
  
  if (OPS_gbl_changed) {
    clSafeCall( clEnqueueWriteBuffer(OPS_opencl_core.command_queue,
      (cl_mem) OPS_consts_d, CL_TRUE, 0, consts_bytes, (void*) OPS_consts_h, 0, NULL, NULL) );
    //clSafeCall( clFlush(OPS_opencl_core.command_queue) );
    clSafeCall( clFinish(OPS_opencl_core.command_queue) );
    memcpy(OPS_gbl_prev,OPS_consts_h,consts_bytes);
  }
}

void mvReductArraysToDevice ( int reduct_bytes )
{
  clSafeCall( clEnqueueWriteBuffer(OPS_opencl_core.command_queue,
    (cl_mem) OPS_reduct_d, CL_TRUE, 0, reduct_bytes, (void*) OPS_reduct_h, 0, NULL, NULL) );
  //clSafeCall( clFlush(OPS_opencl_core.command_queue) );
  clSafeCall( clFinish(OPS_opencl_core.command_queue) );
}

void mvReductArraysToHost ( int reduct_bytes )
{
  clSafeCall( clEnqueueReadBuffer(OPS_opencl_core.command_queue,
    (cl_mem) OPS_reduct_d, CL_TRUE, 0, reduct_bytes, (void*) OPS_reduct_h, 0, NULL, NULL) );
  //clSafeCall( clFlush(OPS_opencl_core.command_queue) );
  clSafeCall( clFinish(OPS_opencl_core.command_queue) );
}


void ops_opencl_exit ( )
{
  if (!OPS_hybrid_gpu) return;
  ops_dat_entry *item;
  TAILQ_FOREACH(item, &OPS_dat_list, entries)
  {
    clReleaseMemObject((cl_mem)(item->dat)->data_d);
  }
  //clSafeCall( clFlush(OPS_opencl_core.command_queue) );
  clSafeCall( clFinish(OPS_opencl_core.command_queue) );
  clSafeCall( clReleaseCommandQueue(OPS_opencl_core.command_queue) );
  clSafeCall( clReleaseContext(OPS_opencl_core.context) );
  free(OPS_opencl_core.platform_id);
}

int ops_is_root()
{
  return 1;
}

void ops_set_halo_dirtybit(ops_arg *arg)
{
  (void)arg;
}

void ops_set_halo_dirtybit3(ops_arg *arg, int *iter_range) {}

void ops_exchange_halo(ops_arg* arg, int d)
{
  (void)arg;
}

void ops_exchange_halo2(ops_arg* arg, int* d_pos, int* d_neg /*depth*/) {}
void ops_exchange_halo3(ops_arg* arg, int* d_pos, int* d_neg /*depth*/, int *iter_range) {}
void ops_halo_exchanges(ops_arg* args, int nargs, int *range) {}


void ops_mpi_reduce_float(ops_arg* args, float* data)
{
  (void)args;
  (void)data;
}

void ops_mpi_reduce_double(ops_arg* args, double* data)
{
  (void)args;
  (void)data;
}

void ops_mpi_reduce_int(ops_arg* args, int* data)
{
  (void)args;
  (void)data;
}


