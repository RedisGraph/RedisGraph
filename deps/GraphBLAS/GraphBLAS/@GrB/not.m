function C = not (G)
%~ logical negation of a GraphBLAS matrix.
% C = ~G computes the logical negation of a GraphBLAS matrix G.  The
% result C is dense, and the computation takes O(m*n) time and space, so
% sparsity is not exploited.  To negate just the entries in the pattern
% of G, use C = GrB.apply ('~.logical', G), which has the same pattern as
% G.
%
% See also GrB.apply.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = GrB.apply ('~.logical', full (G)) ;

