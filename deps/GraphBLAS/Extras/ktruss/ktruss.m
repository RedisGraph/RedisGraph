function [C,nsteps] = ktruss (A,k)
%KTRUSS k-truss of a graph.
%
% C = ktruss (A,k)
%
% A is the adjacency matrix of a graph, and must be square, symmetric, binary,
% and with a zero-free diagonal.  These conditions are not checked.
%
% k defines the k-truss to find.  k >= 3 is required.
%
% C is the k-truss subgraph of A.  Its edges are a subset of A.  Each edge in C
% is part of at least k-2 triangles in C.  The pattern of C, (that is,
% spones(C)), is the adjacency matrix of the k-truss subgraph of A.  The edge
% weights of C are the support of each edge.  That is, C(i,j)=nt if the edge
% (i,j) is part of nt triangles in C.  All edges in C have support of at least
% k-2.  The total number of triangles in C is sum(sum(C))/6.  The number of
% edges in C is nnz(C)/2.  C is returned as symmetric with a zero-free
% diagonal, with all entries greater than or equal to k-2.
%
% The 2nd optional output [C,nsteps] = ktruss (...), returns the # of steps
% the algorithm performed.

%-------------------------------------------------------------------------------
% initializations
%-------------------------------------------------------------------------------

if (nargin < 2)
    % default is 3-truss: each edge of C is in at least one triangle
    k = 3 ;
end

% ensure k is 3 or more
if (k < 3)
    error ('k must be >= 3')
end

% each edge of C must be incident on k-2 triangles
support = k-2 ;

last_cnz = nnz (A) ;
C = A ;

%-------------------------------------------------------------------------------
% find the k-truss of A
%-------------------------------------------------------------------------------

nsteps = 1 ;
while (1)

    % Use the Burkhardt method to count the triangles (see tricount.m), where
    % C(i,j) = # of triangles incident on edge (i,j).  Each triangle is counted
    % 6 times.  Suppose there is a triangle involving edges (i,j,k).  Then it
    % is counted for each 3 choose 2: (i,j), (j,i), (i,k), (k,i), (j,k), (k,j).
    % Note that (i,j) can be an edge in the graph, but if it is not incident on
    % any triangles, then the (i,j)th entry of C*C is zero, so it gets dropped.
    % This step requires C (on input) to be binary.
    C = (C*C) .* C ;

    % The support of the edge (i,j) is C(i,j), which is the # of triangles
    % incident on that edge.  This is the number of triangles that contain the
    % edge (i,j).  The next step removes edges with support less than k-2.
    % If k=3 this step is not needed, since edges with no suport have already
    % been removed by the step above (MATLAB removes all explicit zeros).
    if (k > 3)
        C = C .* (C >= support) ;
    end

    % If no edges have been removed by the two statements above, then C has
    % converged on the k-truss subgraph of the input matrix A.
    cnz = nnz (C) ;
    if (cnz == last_cnz)
        return ;
    end

    % otherwise, continue with the new graph, but make it binary first
    C = spones (C) ;
    last_cnz = cnz ;
    nsteps = nsteps + 1 ;

end

