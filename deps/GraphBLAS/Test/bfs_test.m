function v = bfs_test (A, s)
%BFS_TEST compares bfs_matlab and GB_mex_bfs

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin < 2)
    s = 1 ;
end
A = sparse (A) ;

% test the MATLAB version
tic 
v = bfs_matlab (A, s) ;
t1 = toc ;
n = size (A,1) ;

fprintf ('# of nodes in graph: %d\n', n) ;
fprintf ('source node: %d\n', s) ;
fprintf ('number of levels: %d\n', max (v)) ;
fprintf ('reachable nodes (incl. source): %d\n', length (find (v > 0))) ;

for level = 1:n
    q = find (v == level)' ;
    if (isempty (q))
        break ;
    end
    if (n > 100)
        fprintf ('# of nodes in level %d: %d\n', level, length (q)) ;
    else
        fprintf ('nodes in level %d: ', level) ;
        fprintf ('%d ', q) ;
        fprintf ('\n') ;
    end
end

% now try GB_mex_bfs:

fprintf ('\ntesting GB_mex_bfs:\n') ;

tic
v2 = GB_mex_bfs (A, s) ;
t2 = toc ;

fprintf ('MATLAB    time: %g\n', t1) ;
fprintf ('GraphBLAS time: %g (Demo/bfs5m)\n', t2) ;

assert (isequal (full (v), full (v2))) ;

fprintf ('bfs_test: all tests passed\n') ;


