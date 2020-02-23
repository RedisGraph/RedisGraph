function C = offdiag (A)
%GRB.OFFDIAG removes diaogonal entries from the matrix A.
% C = GrB.offdiag (A) removes diagonal entries from A.
%
% See also tril, triu, diag, GrB.select.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = GrB.select ('offdiag', A, 0) ;

