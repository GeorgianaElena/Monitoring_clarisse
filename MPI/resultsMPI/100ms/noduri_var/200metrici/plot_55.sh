gnuplot -persist -e "set terminal png; set title 'MPI vs EVPATH'; 
set output '55'; 
set xlabel 'Event timestamp'; 
set ylabel 'Propagation Time (sec)'; 
plot 
'./evpath/results_55nodes_2degree_50' title 'EVPATH 55 Nodes' smooth unique with lines,
'res_55nodes_2degree_50file200.txt' title 'MPI 55 Nodes' smooth unique with lines;
unset output; unset terminal";
