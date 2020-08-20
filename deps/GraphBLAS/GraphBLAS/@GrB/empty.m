function C = empty (arg1, arg2)
%GRB.EMPTY construct an empty GraphBLAS sparse matrix.
% C = GrB.empty is a 0-by-0 empty matrix.
% C = GrB.empty (m) is an m-by-0 empty matrix.
% C = GrB.empty ([m n]) or GrB.empty (m,n) is an m-by-n empty matrix, where
% one of m or n must be zero.
%
% All matrices are constructed with the 'double' type.  Use GrB (m,n,type)
% to construct empty single, int*, uint*, and logical m-by-n matrices.
%
% See also GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

m = 0 ;
n = 0 ;
if (nargin == 1)
    if (length (arg1) == 1)
        m = 0 ;
        n = arg1 (1) ;
    elseif (length (arg1) == 2)
        m = arg1 (1) ;
        n = arg1 (2) ;
    else
        gb_error ('invalid dimensions') ;
    end
else
    m = arg1 ;
    n = arg2 ;
end
m = max (m, 0) ;
n = max (n, 0) ;
if (~ ((m == 0) || (n == 0)))
    gb_error ('at least one dimension must be zero') ;
end

C = GrB (m, n) ;

