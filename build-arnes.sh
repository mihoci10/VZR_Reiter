#!/bin/sh

mkdir -p out

echo "Building sequential..."
g++ -o out/ReiterSequential -Wall ReiterSequential.cpp ReiterSim.cpp -Wl,-rpath,./lib -L./lib -l:"libfreeimage.so.3"

echo "Building OpenMP..."
g++ --openmp -o out/ReiterOpenMP -Wall ReiterOpenMP.cpp ReiterSim.cpp -Wl,-rpath,./lib -L./lib -l:"libfreeimage.so.3"

echo "Building CUDA..."
module load CUDA
nvcc ReiterCUDA.cu ReiterSim.cpp -O2 -o out/ReiterCUDA -Xlinker -rpath=./lib -L./lib -l:"libfreeimage.so.3"

echo "Building MPI..."
module load mpi/openmpi-4.1.3
srun --reservation=fri-vr --partition=gpu mpic++ -o out/ReiterMPI -Wall ReiterMPI.cpp ReiterSim.cpp -Xlinker -rpath=./lib -L./lib -l:"libfreeimage.so.3"

echo "Build success!"