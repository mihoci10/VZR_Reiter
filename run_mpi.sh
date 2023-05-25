#!/bin/sh

bash build.sh && srun --reservation=fri --mpi=pmix -n 2 -N 1 out/ReiterMPI 100 100 1 0.5 0.01