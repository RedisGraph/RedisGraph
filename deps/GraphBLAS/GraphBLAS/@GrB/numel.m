function s = numel (G)
%NUMEL the maximum number of entries a GraphBLAS matrix can hold.
% numel (G) is m*n for the m-by-n GraphBLAS matrix G.
% If m, n, or m*n exceed flintmax (2^53), the result is returned as a vpa
% symbolic value, to avoid integer overflow.
%
% See also nnz.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m, n] = size (G) ;
s = m*n ;

if (m > flintmax || n > flintmax || s > flintmax)
    s = vpa (vpa (m, 64) * vpa (n, 64), 64) ;
end

