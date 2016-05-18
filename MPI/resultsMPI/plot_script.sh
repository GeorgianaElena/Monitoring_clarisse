gnuplot -persist -e "set terminal png; set title 'Results for 150 events, 55 nodes, 100ms EVPath vs. MPI'; 
set output 'metrics_var'; 
set xlabel 'Event timestamp'; 
set ylabel 'Propagation Time (sec)'; 
'./evpath/results_55nodes_2degree_150file200.txt' title 'EVPath 200 Metrics' smooth unique with lines,
'./evpath/results_55nodes_2degree_150file500.txt' title 'EVPath 400 Metrics' smooth unique with lines, 
'./evpath/results_55nodes_2degree_150file500.txt' title 'EVPath 500 Metrics' smooth unique with lines;
'res_55nodes_2degree_150file200.txt' title 'MPI 200 Metrics' smooth unique with lines,
'res_55nodes_2degree_150file500.txt' title 'MPI 400 Metrics' smooth unique with lines, 
'res_55nodes_2degree_150file500.txt' title 'MPI 500 Metrics' smooth unique with lines;
unset output; unset terminal";
