function [r, niter] = gap_pagerank (A, d)
%GAP_PAGERANK PageRank of a graph (GAP benchmark algorithm)
% r = gap_pagerank (A) computes the PageRank of a graph with adjacency matrix
% A.  This method uses the same algorithm as the GAP pagerank.  d on input
% is the vector of out degrees, where d(i) = nnz(A(i,:)).  Sinks are ignored in
% the GAP benchmark, so d(i) should be set to 1 if nnz(A(i,:)) is zero.
%
% A can be a GraphBLAS or MATLAB matrix, and must be stored by column.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% set options
tol = 1e-4 ;
maxit = 100 ;
damp = 0.85 ;
type = 'single' ;

n = size (A, 1) ;

% native, if A is already stored by column
native = GrB.isbycol (A) ;
if (~native)
    error ('wrong matrix') ;
end

if (nargin < 2)
    td = tic ;
    d = GrB.entries (A, 'row', 'degree') ;
    sinks = find (d == 0) ;
    if (length (sinks) > 0)
        d (sinks) = 1 ;
    end
    d = GrB (d, 'single') ;
    t = toc (td) ;
    fprintf ('degree time: %g\n', t) ;
end

% teleport factor
tfactor = cast ((1 - damp) / n, type) ;

% sink factor
dn = cast (damp / n, type) ;

% use A' in GrB.mxm
desc.in0 = 'transpose' ;

% initial PageRank: all nodes have rank 1/n
r = GrB (ones (n, 1, type) / n) ;

% prescale d with damp so it doesn't have to be done in each iteration
d = d / damp ;

% compute the PageRank
for iter = 1:maxit
    prior = r ;
    % r(:) = tfactor
    r = GrB.expand (tfactor, r) ;
    %i t = prior ./ d
    t = prior ./ d ;
    % r = r + A' * (prior./d)
    r = GrB.mxm (r, '+', A, '+.2nd.single', t, desc) ;
    % e = norm (r-prior,1)
    e = GrB.normdiff (r, prior, 1) ;
    if (e < tol)
        niter = iter ;
        break ;
    end
end

