#!/bin/sh

bash build.sh

export OMP_PLACES=cores
export OMP_PROC_BIND=TRUE

srun --reservation=fri ReiterSequential

export OMP_NUM_THREADS=32
srun --cpus-per-task=32 --reservation=fri ReiterOpenMP