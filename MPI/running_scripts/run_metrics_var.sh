#!/bin/bash

mpiexec -n 31 ../metrics_aggregator ../input_files/file.txt 150 2 500000
mpiexec -n 31 ../metrics_aggregator ../input_files/file50.txt 150 2 500000
mpiexec -n 31 ../metrics_aggregator ../input_files/file100.txt 150 2 500000
mpiexec -n 31 ../metrics_aggregator ../input_files/file200.txt 150 2 500000
mpiexec -n 31 ../metrics_aggregator ../input_files/file300.txt 150 2 500000
mpiexec -n 31 ../metrics_aggregator ../input_files/file400.txt 150 2 500000
mpiexec -n 31 ../metrics_aggregator ../input_files/file500.txt 150 2 500000
