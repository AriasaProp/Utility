set terminal pngcairo size 900,600 enhanced font 'Verdana,10'
set output 'lab/sort_performance.png'

set title 'Sorting Algorithm Performance Comparison' font 'Verdana,14,Bold'
set xlabel 'Data Variation'
set ylabel 'Execution Time (ms)'

set grid xtics ytics ls 12 lc rgb '#e1e1e1'
set border 3
set tics nomirror
set key outside right center title 'Sort Algorithm' box
set key autotitle columnhead

set autoscale fix

set style line 1 lt 1 lw 2 pt 7  lc rgb '#1f77b4' # Quick
set style line 2 lt 1 lw 2 pt 5  lc rgb '#ff7f0e' # Insertion
set style line 3 lt 1 lw 2 pt 9  lc rgb '#2ca02c' # Selection
set style line 4 lt 1 lw 2 pt 11 lc rgb '#d62728' # Heap
set style line 5 lt 1 lw 2 pt 13 lc rgb '#9467bd' # Merge
set style line 6 lt 1 lw 2 pt 15 lc rgb '#8c564b' # Shell
set style line 7 lt 1 lw 2 pt 17 lc rgb '#e377c2' # Intro
set style line 8 lt 1 lw 2 pt 19 lc rgb '#7f7f7f' # Intro2
set style line 9 lt 1 lw 2 pt 21 lc rgb '#bcbd22' # Intro3

$Data << EOD
L qsort shell insert brick intro merge heap stooge
Sorted 100 100 100 100 100 100 100 100
LessRandom 200 200 200 200 200 200 200 200
HalfRandom 300 300 300 300 300 300 300 300
FirstHalfRandom 400 400 400 400 400 400 400 400
LastHalfRandom 500 500 500 500 500 500 500 500
Random 600 600 600 600 600 600 600 600
Fliped 700 700 700 700 700 700 700 700
EOD

plot for [col=2:15] $Data using col:xtic(1) with linespoints ls (col) title columnhead(col)