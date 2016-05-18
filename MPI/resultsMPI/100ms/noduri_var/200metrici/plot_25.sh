gnuplot -persist -e "set terminal png; set title 'MPI vs EVPATH'; 
set output '25'; 
set xlabel 'Event timestamp'; 
set ylabel 'Propagation Time (sec)'; 
plot 
'./evpath/results_25nodes_2degree_50' title 'EVPATH 25 Nodes' smooth unique with lines, 
'res_25nodes_2degree_50file200.txt' title 'MPI 25 Nodes' smooth unique with lines;
unset output; unset terminal";
