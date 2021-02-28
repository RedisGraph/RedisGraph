function s = ismatrix (G)    %#ok<INUSD>
%ISMATRIX always true for any GraphBLAS matrix.
% ismatrix (G) is always true for any GraphBLAS matrix G.
%
% See also issparse, isvector, isscalar, sparse, full, isa, GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

s = true ;

