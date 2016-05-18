#!/bin/bash

mpiexec -n 5 ../metrics_aggregator ../input_files/file400.txt 150 2 500000
mpiexec -n 10 ../metrics_aggregator ../input_files/file400.txt 150 2 500000
mpiexec -n 20 ../metrics_aggregator ../input_files/file400.txt 150 2 500000
mpiexec -n 30 ../metrics_aggregator ../input_files/file400.txt 150 2 500000
mpiexec -n 40 ../metrics_aggregator ../input_files/file400.txt 150 2 500000
mpiexec -n 50 ../metrics_aggregator ../input_files/file400.txt 150 2 500000
mpiexec -n 55 ../metrics_aggregator ../input_files/file400.txt 150 2 500000