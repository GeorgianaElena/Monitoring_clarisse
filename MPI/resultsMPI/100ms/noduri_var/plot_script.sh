gnuplot -persist -e "set terminal png; set title 'MPI vs EVPATH'; 
set output 'nodes_variable'; 
set xlabel 'Event timestamp'; 
set ylabel 'Propagation Time (sec)'; 
plot './evpath/results_5nodes_2degree_50' title 'EVPATH 5 Nodes' smooth unique with lines, 
'./evpath/results_15nodes_2degree_50' title 'EVPATH 15 Nodes' smooth unique with lines, 
'./evpath/results_25nodes_2degree_50' title 'EVPATH 25 Nodes' smooth unique with lines, 
'./evpath/results_35nodes_2degree_50' title 'EVPATH 35 Nodes' smooth unique with lines, 
'./evpath/results_45nodes_2degree_50' title 'EVPATH 45 Nodes' smooth unique with lines,
'./evpath/results_55nodes_2degree_50' title 'EVPATH 55 Nodes' smooth unique with lines,
'res_5nodes_2degree_50file.txt' title 'MPI 5 Nodes' smooth unique with lines, 
'res_15nodes_2degree_50file.txt' title 'MPI 15 Nodes' smooth unique with lines, 
'res_25nodes_2degree_50file.txt' title 'MPI 25 Nodes' smooth unique with lines, 
'res_35nodes_2degree_50file.txt' title 'MPI 35 Nodes' smooth unique with lines, 
'res_45nodes_2degree_50file.txt' title 'MPI 45 Nodes' smooth unique with lines,
'res_55nodes_2degree_50file.txt' title 'MPI 55 Nodes' smooth unique with lines; 
unset output; unset terminal";
