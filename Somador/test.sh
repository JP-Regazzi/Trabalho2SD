#!/bin/bash

n=1000000000
k=1

for i in {1..9}
do
    echo "Running program with n=$n and k=$k"
    ./somador $n $k
    k=$((k*2))
done