gnuplot -persist -e "set terminal png; set title 'Results for 50 events, 55 nodes, 100ms'; 
set output 'metrics_var'; 
set xlabel 'Event timestamp'; 
set ylabel 'Propagation Time (sec)'; 
plot 'results_55nodes_2degree_150file.txt' title '10 Metrics' smooth unique with lines, 'results_55nodes_2degree_150file50.txt' title '50 Metrics' smooth unique with lines, 
'results_55nodes_2degree_150file100.txt' title '100 Metrics' smooth unique with lines, 'results_55nodes_2degree_150file200.txt' title '200 Metrics' smooth unique with lines, 
'results_55nodes_2degree_150file300.txt' title '300 Metrics' smooth unique with lines, 'results_55nodes_2degree_150file400.txt' title '400 Metrics' smooth unique with lines, 
'results_55nodes_2degree_150file500.txt' title '500 Metrics' smooth unique with lines;
unset output; unset terminal";
