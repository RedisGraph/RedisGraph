function n = length (G)
%LENGTH the length of a GraphBLAS vector.
% length (G) is the length of the vector G.  For matrices, it is
% max (size (G)) if G is non-empty, or zero if G has any zero dimension.
% If any dimension of G exceeds flintmax, the result is returned as int64
% to avoid integer overflow.
%
% See also size, numel.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m, n] = size (G) ;

if (m == 0 || n == 0)
    n = 0 ;
else
    n = max (m, n) ;
end

