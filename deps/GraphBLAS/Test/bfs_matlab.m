function v = bfs_matlab (A, s)
%BFS_MATLAB a simple breadth-first-search in MATLAB
%
% v = bfs_matlab (A, s)
%
% A is a square binary matrix, corresponding to the adjacency matrix of a
% graph, with A(i,j)=1 denoting the edge (i,j).  Self loops are permitted, and
% A may be unsymmetric.  s is a scalar input with the source node.  The output
% v is the level each node in the graph, where v(s)=1 (the first level), v(j)=2
% if there is an edge (s,j) (the 2nd level), etc.  v(j)=k if node j is in the
% kth level, where the shortest path (in terms of # of edges) from  s to j has
% length k+1.  The source node s defaults to 1.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m n] = size (A) ;
if (m ~= n)
    error ('A must be square') ;
end
n = size (A,1) ;
v = zeros (n,1) ;

if (nargin < 2)
    s = 1 ;
end

% ensure A is binary, and transpose it
AT = spones (A') ;

q = zeros (n,1) ;
q (s) = 1 ;         % q is the current level

for level = 1:n

    % assign the level to all nodes in q
    v (q ~= 0) = level ;

    % find all neighbors of q, as a binary vector
    qnew = spones (AT * q) ;

    % discard nodes in qnew that are already seen
    qnew (v ~= 0) = 0 ;

    % move to the new level
    q = qnew ;

    % stop if the new level is empty
    if (~any (q))
        break ;
    end

end

