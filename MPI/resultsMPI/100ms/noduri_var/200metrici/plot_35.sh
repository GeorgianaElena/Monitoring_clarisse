gnuplot -persist -e "set terminal png; set title 'MPI vs EVPATH'; 
set output '35'; 
set xlabel 'Event timestamp'; 
set ylabel 'Propagation Time (sec)'; 
plot 
'./evpath/results_35nodes_2degree_50' title 'EVPATH 35 Nodes' smooth unique with lines, 
'res_35nodes_2degree_50file200.txt' title 'MPI 35 Nodes' smooth unique with lines;
unset output; unset terminal";
