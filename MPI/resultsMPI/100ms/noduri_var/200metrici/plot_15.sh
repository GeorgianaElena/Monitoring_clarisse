gnuplot -persist -e "set terminal png; set title 'MPI vs EVPATH'; 
set output '15'; 
set xlabel 'Event timestamp'; 
set ylabel 'Propagation Time (sec)'; 
plot './evpath/results_15nodes_2degree_50' title 'EVPATH 15 Nodes' smooth unique with lines, 
'res_15nodes_2degree_50file200.txt' title 'MPI 15 Nodes' smooth unique with lines;
unset output; unset terminal";
