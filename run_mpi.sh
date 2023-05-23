#!/bin/sh
bash build.sh && srun --reservation=fri --mpi=pmix -n 4 -N 1 out/ReiterMPI