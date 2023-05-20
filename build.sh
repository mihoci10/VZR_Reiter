#!/bin/sh

g++ -o ReiterSequential -Wall ReiterSequential.cpp ReiterSim.cpp -Wl,-rpath,./lib -L./lib -l:"libfreeimage.so.3"