function [r,irank] = ipagerank (A) ;
%IPAGERANK compute the pagerank of nodes in a graph using an integer semiring
% Usage:
% [r,irank] = ipagerank (A) ;
%
% A is a square unsymmetric binary sparse matrix of size n-by-n, where A(i,j)
% is the edge (i,j) from page i to page j.  Self-edges are OK.  On output,
% irank is a permutation of 1:n with irank(1) being the top ranked page,
% irank(2) is the 2nd ranked page, and so on.  r is the pagerank of the nodes,
% where r(k) is pagerank of page irank(k).
%
% See also dpagerank.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% original problem in real arithmetic
n = size (A,1) ;        % number of nodes
c = 0.85 ;              % probability of walking to random neighbor
r = rand (1,n) ;        % random initial pageranks
r = r / sum (r) ;       % normalize so sum(r) == 1
a = (1-c) / n ;         % to jump to any random node in entire graph
C = rowscale (A) ;      % scale A by out-degree

% convert problem from real to integer
z = 2^30 ;              % multiplicative constant to scale real to integer
r = floor (r * z) ;
a = floor (a * z) ;
c = floor (c * z) ;
C = floor (C * z) ;

% iterate to compute the pagerank of each node
for i = 1:20
    r = floor ((floor (floor ((c*r)/z) * C) + floor (a * sum (r))) / z) ;
end

% sort the nodes by pagerank
[r,irank] = sort (r, 'descend') ;
