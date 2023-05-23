#!/bin/sh

g++ -o ReiterSequential -Wall ReiterSequential.cpp ReiterSim.cpp -Wl,-rpath,./lib -L./lib -l:"libfreeimage.so.3"
g++ --openmp -o ReiterOpenMP -Wall ReiterOpenMP.cpp ReiterSim.cpp -Wl,-rpath,./lib -L./lib -l:"libfreeimage.so.3"

module load CUDA/10.1.243-GCC-8.3.0
nvcc ReiterCUDA.cu ReiterSim.cpp -O2 -o ReiterCUDA -Xlinker -rpath=./lib -L./lib -l:"libfreeimage.so.3"



