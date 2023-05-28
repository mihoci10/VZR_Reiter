#!/bin/sh

mkdir -p benchmarks
LOG="benchmarks/$(date -d "today" +"%Y-%m-%d-%H:%M:%S").log"
touch $LOG

echo "[" > $LOG

bash run-arnes.sh $LOG 10 10 1 0.5 0.01

echo "]" >> $LOG    

grep -v "ERROR" $LOG > benchmarks/tmpfile && mv benchmarks/tmpfile $LOG

exit