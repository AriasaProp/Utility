file = "data/algorithm/sort_report.txt"

set terminal pngcairo size 2100,1200 enhanced font 'Verdana,10'
set output 'data/algorithm/sort_report.png'

set title "Sorter" font 'Verdana,14,Bold'
set xlabel 'Data Variation'
set ylabel 'Execution Time (ms)'

set grid xtics ytics ls 12 lc rgb '#e1e1e1'
set border 3
set tics nomirror
set key outside right center title 'Sort Algorithm' box
set key autotitle columnhead

set autoscale fix
N = 5
M = 5.0
do for [n=1:N] {
    set style line n lt 1 lw 2 pt n lc rgb hsv2rgb( (n-1)/M, 1, 1 )
}

plot for [col=2:(N+1)] file using col:xtic(1) with linespoints ls (col - 1) title columnhead(col)
