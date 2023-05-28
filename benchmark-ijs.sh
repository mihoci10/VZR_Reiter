#!/bin/sh

bash build-ijs.sh

mkdir -p benchmarks
LOG="benchmarks/$(date -d "today" +"%Y-%m-%d-%H:%M:%S").log"
touch $LOG

echo "[" > $LOG

bash run-ijs.sh $LOG 100 100 0.502 0.4 0.0001

echo "]" >> $LOG    

grep -v "ERROR" $LOG > benchmarks/tmpfile && mv benchmarks/tmpfile $LOG

exit