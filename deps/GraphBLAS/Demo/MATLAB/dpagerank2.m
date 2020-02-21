function [r,irank,iters] = dpagerank2 (A, tol, itermax)
%DPAGERANK2 compute the pagerank of nodes in a graph using a real semiring
% Usage:
% [r,irank,iters] = dpagerank2 (A) ;
%
% A is a square unsymmetric binary sparse matrix of size n-by-n, where A(i,j)
% is the edge (i,j) from page i to page j.  Self-edges are OK.  On output,
% irank is a permutation of 1:n with irank(1) being the top ranked page,
% irank(2) is the 2nd ranked page, and so on.  r is the pagerank of the nodes,
% where r(k) is pagerank of page irank(k).
%
% iters is the number of iterations taken.
%
% Two additional arguments can be provided:
%
% [r,irank,iters] = dpagerank2 (A, tol, itermax) ;
%
% tol is the stopping criterion (default is 1e-5).  The iteration stops
% if the 2norm of r changes by < 1e-5.  itermax (default is 100) is the
% max number of iterations allows.
%
% See also ipagerank.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin < 2)
    tol = 1e-5 ;        % stopping criterion
end

if (nargin < 3)
    itermax = 100 ;     % max # of iterations
end

% original problem in real arithmetic
n = size (A,1) ;        % number of nodes
c = 0.85 ;              % probability of walking to random neighbor
r = ones (1,n) / n ;    % initial uniform probability
% r = rand (1,n) ;        % random initial pageranks
% r = r / sum (r) ;       % normalize so sum(r) == 1
a = (1-c) / n ;         % to jump to any random node in entire graph
C = c * rowscale (A) ;  % scale A by out-degree and damping factor

% iterate to compute the pagerank of each node
for iters = 1:itermax
    rnew = r * C + a ;
    if (norm (r-rnew) <= tol)
        break ;
    end
    r = rnew ;
end

r = r / sum (r) ;       % normalize r so sum(r)==1

% sort the nodes by pagerank
[r,irank] = sort (r, 'descend') ;

