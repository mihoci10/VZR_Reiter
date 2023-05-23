#!/bin/sh

bash build.sh

srun --reservation=fri ReiterSequential

export OMP_PLACES=cores
export OMP_PROC_BIND=TRUE
export OMP_NUM_THREADS=32
srun --cpus-per-task=32 --reservation=fri ReiterOpenMP

srun --reservation=fri -G1 -n1 ReiterCUDA