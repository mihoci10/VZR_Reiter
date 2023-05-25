#!/bin/sh

bash build.sh && srun --reservation=fri --mpi=pmix -n 16 -N 2 out/ReiterMPI 100 100 1 0.5 0.01