//
// auto-generated by ops.py
//

extern int xdim0_initialise_chunk_kernel_x;
int xdim0_initialise_chunk_kernel_x_h = -1;
extern int ydim0_initialise_chunk_kernel_x;
int ydim0_initialise_chunk_kernel_x_h = -1;
extern int xdim1_initialise_chunk_kernel_x;
int xdim1_initialise_chunk_kernel_x_h = -1;
extern int ydim1_initialise_chunk_kernel_x;
int ydim1_initialise_chunk_kernel_x_h = -1;
extern int xdim2_initialise_chunk_kernel_x;
int xdim2_initialise_chunk_kernel_x_h = -1;
extern int ydim2_initialise_chunk_kernel_x;
int ydim2_initialise_chunk_kernel_x_h = -1;

#ifdef __cplusplus
extern "C" {
#endif
void initialise_chunk_kernel_x_c_wrapper(
  double *p_a0,
  int *p_a1,
  double *p_a2,
  int x_size, int y_size, int z_size);

#ifdef __cplusplus
}
#endif

// host stub function
void ops_par_loop_initialise_chunk_kernel_x(char const *name, ops_block block, int dim, int* range,
 ops_arg arg0, ops_arg arg1, ops_arg arg2) {

  ops_arg args[3] = { arg0, arg1, arg2};


  #ifdef CHECKPOINTING
  if (!ops_checkpointing_before(args,3,range,3)) return;
  #endif

  if (OPS_diags > 1) {
    ops_timing_realloc(3,"initialise_chunk_kernel_x");
    OPS_kernels[3].count++;
  }

  //compute localy allocated range for the sub-block
  int start[3];
  int end[3];
  int arg_idx[3];

  #ifdef OPS_MPI
  sub_block_list sb = OPS_sub_block_list[block->index];
  if (compute_ranges(args, 3,block, range, start, end, arg_idx) < 0) return;
  #else
  for ( int n=0; n<3; n++ ){
    start[n] = range[2*n];end[n] = range[2*n+1];
    arg_idx[n] = start[n];
  }
  #endif

  int x_size = MAX(0,end[0]-start[0]);
  int y_size = MAX(0,end[1]-start[1]);
  int z_size = MAX(0,end[2]-start[2]);

  int xdim0 = args[0].dat->size[0];
  int ydim0 = args[0].dat->size[1];
  int xdim1 = args[1].dat->size[0];
  int ydim1 = args[1].dat->size[1];
  int xdim2 = args[2].dat->size[0];
  int ydim2 = args[2].dat->size[1];

  //Timing
  double t1,t2,c1,c2;
  if (OPS_diags > 1) {
    ops_timers_core(&c2,&t2);
  }

  if (xdim0 != xdim0_initialise_chunk_kernel_x_h || ydim0 != ydim0_initialise_chunk_kernel_x_h || xdim1 != xdim1_initialise_chunk_kernel_x_h || ydim1 != ydim1_initialise_chunk_kernel_x_h || xdim2 != xdim2_initialise_chunk_kernel_x_h || ydim2 != ydim2_initialise_chunk_kernel_x_h) {
    xdim0_initialise_chunk_kernel_x = xdim0;
    xdim0_initialise_chunk_kernel_x_h = xdim0;
    ydim0_initialise_chunk_kernel_x = ydim0;
    ydim0_initialise_chunk_kernel_x_h = ydim0;
    xdim1_initialise_chunk_kernel_x = xdim1;
    xdim1_initialise_chunk_kernel_x_h = xdim1;
    ydim1_initialise_chunk_kernel_x = ydim1;
    ydim1_initialise_chunk_kernel_x_h = ydim1;
    xdim2_initialise_chunk_kernel_x = xdim2;
    xdim2_initialise_chunk_kernel_x_h = xdim2;
    ydim2_initialise_chunk_kernel_x = ydim2;
    ydim2_initialise_chunk_kernel_x_h = ydim2;
  }


  int dat0 = (OPS_soa ? args[0].dat->type_size : args[0].dat->elem_size);
  int dat1 = (OPS_soa ? args[1].dat->type_size : args[1].dat->elem_size);
  int dat2 = (OPS_soa ? args[2].dat->type_size : args[2].dat->elem_size);

  //set up initial pointers and exchange halos if necessary
  int base0 = args[0].dat->base_offset + (OPS_soa ? args[0].dat->type_size : args[0].dat->elem_size) * start[0] * args[0].stencil->stride[0];
  base0 = base0+ (OPS_soa ? args[0].dat->type_size : args[0].dat->elem_size) *
    args[0].dat->size[0] *
    start[1] * args[0].stencil->stride[1];
  base0 = base0+ (OPS_soa ? args[0].dat->type_size : args[0].dat->elem_size) *
    args[0].dat->size[0] *
    args[0].dat->size[1] *
    start[2] * args[0].stencil->stride[2];
  double *p_a0 = (double *)(args[0].data + base0);

  int base1 = args[1].dat->base_offset + (OPS_soa ? args[1].dat->type_size : args[1].dat->elem_size) * start[0] * args[1].stencil->stride[0];
  base1 = base1+ (OPS_soa ? args[1].dat->type_size : args[1].dat->elem_size) *
    args[1].dat->size[0] *
    start[1] * args[1].stencil->stride[1];
  base1 = base1+ (OPS_soa ? args[1].dat->type_size : args[1].dat->elem_size) *
    args[1].dat->size[0] *
    args[1].dat->size[1] *
    start[2] * args[1].stencil->stride[2];
  int *p_a1 = (int *)(args[1].data + base1);

  int base2 = args[2].dat->base_offset + (OPS_soa ? args[2].dat->type_size : args[2].dat->elem_size) * start[0] * args[2].stencil->stride[0];
  base2 = base2+ (OPS_soa ? args[2].dat->type_size : args[2].dat->elem_size) *
    args[2].dat->size[0] *
    start[1] * args[2].stencil->stride[1];
  base2 = base2+ (OPS_soa ? args[2].dat->type_size : args[2].dat->elem_size) *
    args[2].dat->size[0] *
    args[2].dat->size[1] *
    start[2] * args[2].stencil->stride[2];
  double *p_a2 = (double *)(args[2].data + base2);



  ops_H_D_exchanges_host(args, 3);
  ops_halo_exchanges(args,3,range);

  if (OPS_diags > 1) {
    ops_timers_core(&c1,&t1);
    OPS_kernels[3].mpi_time += t1-t2;
  }

  initialise_chunk_kernel_x_c_wrapper(
    p_a0,
    p_a1,
    p_a2,
    x_size, y_size, z_size);

  if (OPS_diags > 1) {
    ops_timers_core(&c2,&t2);
    OPS_kernels[3].time += t2-t1;
  }
  ops_set_dirtybit_host(args, 3);
  ops_set_halo_dirtybit3(&args[0],range);
  ops_set_halo_dirtybit3(&args[2],range);

  //Update kernel record
  if (OPS_diags > 1) {
    OPS_kernels[3].transfer += ops_compute_transfer(dim, start, end, &arg0);
    OPS_kernels[3].transfer += ops_compute_transfer(dim, start, end, &arg1);
    OPS_kernels[3].transfer += ops_compute_transfer(dim, start, end, &arg2);
  }
}
