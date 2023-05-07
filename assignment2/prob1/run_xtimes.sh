#!/bin/bash

n_workers=(1 3 5 8)

for value in "${n_workers[@]}"; do
    for ((i=1; i<=10; i++)); do
        # Execute your desired command or line here
        echo "$value"
        echo 'mpiexec -n $(( $value + 1))./main ../../assignment1/dataset/prob1/text{1,2,3,4}.txt'
        echo $value "," $(mpiexec -n $(( $value + 1)) ./main ../../assignment1/dataset/prob1/text{1,2,3,4}.txt | awk '/Elapsed time/ {printf $3 "\n"}' ) >> dados.csv
    done
done
