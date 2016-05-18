gnuplot -persist -e "set terminal png; set title 'MPI vs EVPATH'; 
set output '45'; 
set xlabel 'Event timestamp'; 
set ylabel 'Propagation Time (sec)'; 
plot 
'./evpath/results_45nodes_2degree_50' title 'EVPATH 45 Nodes' smooth unique with lines,
'res_45nodes_2degree_50file200.txt' title 'MPI 45 Nodes' smooth unique with lines;
unset output; unset terminal";
