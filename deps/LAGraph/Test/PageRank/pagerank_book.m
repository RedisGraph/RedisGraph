
% graph on the cover of the book, 'Graph Algorithms in the language
% of linear algebra'.  The source node is node 4.
clear
o = 0 ;

A = [
o o o 1 o o o
1 o o o o o o
o o o 1 o 1 1
1 o o o o o 1
o 1 o o o o 1
o o 1 o 1 o o
o 1 o o o o o ] ;

A = A' ;

[r,irank,iters] = pagerank_test (A)

