#!/bin/sh

mkdir -p out

g++ -o out/ReiterSequential -Wall ReiterSequential.cpp ReiterSim.cpp -Wl,-rpath,./lib -L./lib -l:"libfreeimage.so.3"

g++ --openmp -o out/ReiterOpenMP -Wall ReiterOpenMP.cpp ReiterSim.cpp -Wl,-rpath,./lib -L./lib -l:"libfreeimage.so.3"

module load CUDA/10.1.243-GCC-8.3.0
nvcc ReiterCUDA.cu ReiterSim.cpp -O2 -o out/ReiterCUDA -Xlinker -rpath=./lib -L./lib -l:"libfreeimage.so.3"

module load OpenMPI/4.1.0-GCC-10.2.0
mpic++ -o out/ReiterMPI -Wall ReiterMPI.cpp ReiterSim.cpp -Xlinker -rpath=./lib -L./lib -l:"libfreeimage.so.3"