#!/bin/bash

mpiexec -n 31 ../metrics_aggregator ../input_files/file200.txt 150 2 100000
mpiexec -n 31 ../metrics_aggregator ../input_files/file200.txt 150 4 100000
mpiexec -n 31 ../metrics_aggregator ../input_files/file200.txt 150 8 100000
mpiexec -n 31 ../metrics_aggregator ../input_files/file200.txt 150 16 100000
mpiexec -n 31 ../metrics_aggregator ../input_files/file200.txt 150 30 100000
