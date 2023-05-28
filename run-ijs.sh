#!/bin/sh

echo "Executing run..."
echo "  Width: $2"
echo "  Height: $3"
echo "  Alpha: $4"
echo "  Beta: $5"
echo "  Gamma: $6"

export OMP_PLACES=cores
export OMP_PROC_BIND=TRUE

echo "Executing sequential..."

srun --reservation=fri out/ReiterSequential $2 $3 $4 $5 $6 >> $1

echo "Executing OpenMP... (1 thread)"
export OMP_NUM_THREADS=1
srun --cpus-per-task=1 --reservation=fri out/ReiterOpenMP $2 $3 $4 $5 $6 >> $1

echo "Executing OpenMP... (2 thread)"
export OMP_NUM_THREADS=2
srun --cpus-per-task=2 --reservation=fri out/ReiterOpenMP $2 $3 $4 $5 $6 >> $1

echo "Executing OpenMP... (4 thread)"
export OMP_NUM_THREADS=4
srun --cpus-per-task=4 --reservation=fri out/ReiterOpenMP $2 $3 $4 $5 $6 >> $1

echo "Executing OpenMP... (8 thread)"
export OMP_NUM_THREADS=8
srun --cpus-per-task=8 --reservation=fri out/ReiterOpenMP $2 $3 $4 $5 $6 >> $1

echo "Executing OpenMP... (16 thread)"
export OMP_NUM_THREADS=16
srun --cpus-per-task=16 --reservation=fri out/ReiterOpenMP $2 $3 $4 $5 $6 >> $1

echo "Executing OpenMP... (32 thread)"
export OMP_NUM_THREADS=32
srun --cpus-per-task=32 --reservation=fri out/ReiterOpenMP $2 $3 $4 $5 $6 >> $1

echo "Executing OpenMP... (64 thread)"
export OMP_NUM_THREADS=64
srun --cpus-per-task=64 --reservation=fri out/ReiterOpenMP $2 $3 $4 $5 $6 >> $1

echo "Executing OpenMP... (128 thread)"
export OMP_NUM_THREADS=128
srun --cpus-per-task=128 --reservation=fri out/ReiterOpenMP $2 $3 $4 $5 $6 >> $1

module load CUDA/10.1.243-GCC-8.3.0

echo "Executing CUDA... (1 GPU)"
srun --reservation=fri --gpus=1 out/ReiterCUDA $2 $3 $4 $5 $6 >> $1

echo "Executing CUDA... (2 GPUs)"
srun --reservation=fri --gpus=2 out/ReiterCUDA $2 $3 $4 $5 $6 >> $1


module load OpenMPI/4.1.0-GCC-10.2.0

echo "Executing MPI... (1 runner | 1 node)"
srun --reservation=fri  --mpi=pmix -n 1 -N 1 out/ReiterMPI $2 $3 $4 $5 $6 >> $1

echo "Executing MPI... (2 runner | 1 node)"
srun --reservation=fri  --mpi=pmix -n 2 -N 1 out/ReiterMPI $2 $3 $4 $5 $6 >> $1

echo "Executing MPI... (4 runner | 1 node)"
srun --reservation=fri  --mpi=pmix -n 4 -N 1 out/ReiterMPI $2 $3 $4 $5 $6 >> $1

echo "Executing MPI... (8 runner | 1 node)"
srun --reservation=fri  --mpi=pmix -n 8 -N 1 out/ReiterMPI $2 $3 $4 $5 $6 >> $1

echo "Executing MPI... (16 runner | 1 node)"
srun --reservation=fri  --mpi=pmix -n 16 -N 1 out/ReiterMPI $2 $3 $4 $5 $6 >> $1

echo "Executing MPI... (32 runner | 1 node)"
srun --reservation=fri  --mpi=pmix -n 32 -N 1 out/ReiterMPI $2 $3 $4 $5 $6 >> $1

echo "Executing MPI... (32 runner | 2 node)"
srun --reservation=fri  --mpi=pmix -n 32 -N 2 out/ReiterMPI $2 $3 $4 $5 $6 >> $1

echo "Executing MPI... (64 runner | 1 node)"
srun --reservation=fri  --mpi=pmix -n 64 -N 1 out/ReiterMPI $2 $3 $4 $5 $6 >> $1

echo "Executing MPI... (64 runner | 2 node)"
srun --reservation=fri  --mpi=pmix -n 64 -N 2 out/ReiterMPI $2 $3 $4 $5 $6 >> $1