gnuplot -persist -e "set terminal png; set title 'MPI vs EVPATH'; 
set output '5'; 
set xlabel 'Event timestamp'; 
set ylabel 'Propagation Time (sec)'; 
plot './evpath/results_5nodes_2degree_50' title 'EVPATH 5 Nodes' smooth unique with lines, 
'res_5nodes_2degree_50file200.txt' title 'MPI 5 Nodes' smooth unique with lines;
unset output; unset terminal";
