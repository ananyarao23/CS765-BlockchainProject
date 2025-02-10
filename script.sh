#!/bin/bash

z0=50
z1=50
num_blocks=5

rm out_*
make clean
make all


for I in {100000..600000..100000}; do
    for Ttx in {10000..60000..10000}; do
        echo "Running for I = $I, Ttx = $Ttx"
        ./sim $z0 $z1 50 $((num_blocks * I * 50)) $I $Ttx > out_${I}_${Ttx}.txt
    done
done

