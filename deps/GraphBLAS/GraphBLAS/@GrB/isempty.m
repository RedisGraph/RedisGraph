function s = isempty (G)
%ISEMPTY true for empty GraphBLAS matrix.
% isempty (G) is true if any dimension of G is zero.
%
% See also size.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m, n] = size (G) ;
s = (m == 0) | (n == 0) ;

