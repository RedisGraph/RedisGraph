function C = sparse (G)
%SPARSE make a copy of a GraphBLAS sparse matrix.
% Since G is already sparse, C = sparse (G) simply makes a copy of G.
% Explicit zeros are not removed.  To remove them use C = GrB.prune(G).
%
% See also GrB/issparse, GrB/full, GrB.type, GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = G ;

