#!/bin/sh

bash build.sh

srun --reservation=fri out/ReiterSequential 100 100 1 0.5 0.01

export OMP_PLACES=cores
export OMP_PROC_BIND=TRUE
export OMP_NUM_THREADS=32
srun --cpus-per-task=32 --reservation=fri out/ReiterOpenMP 100 100 1 0.5 0.01

srun --reservation=fri -G1 -n1 out/ReiterCUDA 100 100 1 0.5 0.01