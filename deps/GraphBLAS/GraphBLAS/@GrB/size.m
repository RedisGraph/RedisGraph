function [arg1, n] = size (G, dim)
%SIZE the dimensions of a GraphBLAS matrix.
% [m n] = size (G) is the size of an m-by-n GraphBLAS sparse matrix.
% If any dimension exceeds flintmax (2^53), m and n are returned as int64.
%
% See also GrB/length, GrB/numel.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargout <= 1)
    arg1 = gbsize (G.opaque) ;
    if (nargin == 2)
        arg1 = arg1 (dim) ;
    end
else
    [arg1, n] = gbsize (G.opaque) ;
end

