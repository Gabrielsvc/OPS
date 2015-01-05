#!/bin/bash

cd ../../ops/
source ./source_intel
make
cd -
make clean
make
#============================ Test Cloverleaf 3D ==========================================================
echo '============> Running OpenMP'
KMP_AFFINITY=compact OMP_NUM_THREADS=24 ./cloverleaf_openmp > perf_out
grep "Total Wall time" clover.out
grep "step:   2955" clover.out
echo '============> Running MPI+OpenMP'
export OMP_NUM_THREADS=2;$MPI_INSTALL_PATH/bin/mpirun -np 12 ./cloverleaf_mpi_openmp > perf_out
grep "Total Wall time" clover.out
grep "step:   2955" clover.out
echo '============> Running MPI'
$MPI_INSTALL_PATH/bin/mpirun -np 22 ./cloverleaf_mpi > perf_out
grep "Total Wall time" clover.out
grep "step:   2955" clover.out
echo '============> Running CUDA'
./cloverleaf_cuda OPS_BLOCK_SIZE_X=64 OPS_BLOCK_SIZE_Y=4 > perf_out
grep "Total Wall time" clover.out
grep "step:   2953" clover.out
grep "step:   2954" clover.out
grep "step:   2955" clover.out
echo '============> Running MPI+CUDA'
$MPI_INSTALL_PATH/bin/mpirun -np 2 ./cloverleaf_mpi_cuda OPS_BLOCK_SIZE_X=64 OPS_BLOCK_SIZE_Y=4 > perf_out
grep "Total Wall time" clover.out
grep "step:   2953" clover.out
grep "step:   2954" clover.out
grep "step:   2955" clover.out
echo '============> Running MPI+CUDA with GPU-Direct'
MV2_USE_CUDA=1 $MPI_INSTALL_PATH/bin/mpirun -np 2 ./cloverleaf_mpi_cuda -gpudirect OPS_BLOCK_SIZE_X=64 OPS_BLOCK_SIZE_Y=4 > perf_out
grep "Total Wall time" clover.out
grep "step:   2953" clover.out
grep "step:   2954" clover.out
grep "step:   2955" clover.out
echo '============> Running OpenCL on CPU'
./cloverleaf_opencl OPS_CL_DEVICE=0 OPS_BLOCK_SIZE_X=512 OPS_BLOCK_SIZE_Y=1 > perf_out
grep "Total Wall time" clover.out
grep "step:   2953" clover.out
grep "step:   2954" clover.out
grep "step:   2955" clover.out
echo '============> Running OpenCL on GPU'
./cloverleaf_opencl OPS_CL_DEVICE=1 OPS_BLOCK_SIZE_X=32 OPS_BLOCK_SIZE_Y=4 > perf_out
./cloverleaf_opencl OPS_CL_DEVICE=1 OPS_BLOCK_SIZE_X=32 OPS_BLOCK_SIZE_Y=4 > perf_out
grep "Total Wall time" clover.out
grep "step:   2953" clover.out
grep "step:   2954" clover.out
grep "step:   2955" clover.out
rm perf_out
echo '============> Running MPI+OpenCL on CPU'
$MPI_INSTALL_PATH/bin/mpirun -np 24 ./cloverleaf_mpi_opencl OPS_CL_DEVICE=0  > perf_out
$MPI_INSTALL_PATH/bin/mpirun -np 24 ./cloverleaf_mpi_opencl OPS_CL_DEVICE=0  > perf_out
grep "Total Wall time" clover.out
grep "step:   2953" clover.out
grep "step:   2954" clover.out
grep "step:   2955" clover.out
rm perf_out
echo '============> Running MPI+OpenCL on GPU'
$MPI_INSTALL_PATH/bin/mpirun -np 2 ./cloverleaf_mpi_opencl OPS_CL_DEVICE=1 OPS_BLOCK_SIZE_X=32 OPS_BLOCK_SIZE_Y=4 > perf_out
$MPI_INSTALL_PATH/bin/mpirun -np 2 ./cloverleaf_mpi_opencl OPS_CL_DEVICE=1 OPS_BLOCK_SIZE_X=32 OPS_BLOCK_SIZE_Y=4 > perf_out
grep "Total Wall time" clover.out
grep "step:   2953" clover.out
grep "step:   2954" clover.out
grep "step:   2955" clover.out
rm perf_out




cd -
source source_pgi
make cuda
make mpi_cuda
cd -
make cloverleaf_openacc
make cloverleaf_mpi_openacc
echo '============> Running OpenACC'
./cloverleaf_openacc OPS_BLOCK_SIZE_X=64 OPS_BLOCK_SIZE_Y=4 > perf_out
grep "Total Wall time" clover.out
grep "step:   2953" clover.out
grep "step:   2954" clover.out
grep "step:   2955" clover.out
rm perf_out
echo '============> Running MPI+OpenACC'
$MPI_INSTALL_PATH/bin/mpirun -np 2 ./cloverleaf_mpi_openacc OPS_BLOCK_SIZE_X=64 OPS_BLOCK_SIZE_Y=4 > perf_out
grep "Total Wall time" clover.out
grep "step:   2953" clover.out
grep "step:   2954" clover.out
grep "step:   2955" clover.out
rm perf_out
