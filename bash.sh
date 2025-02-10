#!/bin/bash

# Define parameter ranges (modify as needed)
param1_values=(15 20 25)
param2_values=(15 20 25)
param3_values=(60)
param4_values=(1500000000 3000000000 4500000000)
param5_values=(600000 600000 600000)
param6_values=(10 15 20)

# Loop over all combinations
for p1 in "${param1_values[@]}"; do
    for p2 in "${param2_values[@]}"; do
        for p3 in "${param3_values[@]}"; do
            for p4 in "${param4_values[@]}"; do
                for p5 in "${param5_values[@]}"; do
                    for p6 in "${param6_values[@]}"; do
                        output_file="out_${p1}_${p2}_${p3}_${p4}_${p5}_${p6}.txt"
                        echo "Running ./sim $p1 $p2 $p3 $p4 $p5 $p6 > $output_file"
                        ./sim $p1 $p2 $p3 $p4 $p5 $p6 > "$output_file"
                    done
                done
            done
        done
    done
done
