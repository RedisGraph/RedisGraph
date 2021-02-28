%BFS_BOOK run BFS on a small graph

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

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

% now test all source nodes
n = size (A, 1) ;
for s = 1:n
    fprintf ('\n====================== source: %d\n', s) ;
    v = bfs_test (A, s) ;
    v
end

fprintf ('bfs_book test passed\n') ;

