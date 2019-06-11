
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
s = 4 ;

v = bfs_test (A, s) ;

vok = [2 3 2 1 4 3 4]' ;
assert (isequal (v, vok)) ;

fprintf ('bfs_book test passed\n') ;
