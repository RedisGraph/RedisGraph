function v = bfs_test (A, s, filename) ;

if (nargin < 3)
    filename = 'A.mtx' ;
end
if (nargin < 2)
    s = 1 ;
end

AT = A' ;

% test the MATLAB version
v = bfs_matlab (AT, s) ;
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
    fprintf ('nodes in level %d: ', level) ;
    fprintf ('%d ', q) ;
    fprintf ('\n') ;
end

% now try LAGraph_bfs_simple:

fprintf ('\ntesting LAGraph_bfs_simple:\n') ;

% TODO: for now, this requires mread and mwrite from SuiteSparse/CHOLMOD (see
% http://suitesparse.com)  It would be better to write a MATLAB interface to
% LAGraph_mmread and LAGraph_mmwrite, so this test could be self-contained.

outfile = 'v' ;
mwrite (filename, sparse (A)) ;
% convert s to zero-based
system (sprintf ('./build/bfs_test %d < %s > %s', s-1, filename, outfile)) ; 
v2 = load (outfile) ;

assert (isequal (v, v2)) ;

fprintf ('bfs_test: all tests passed\n') ;

