function C = ldivide (A, B)
%LDIVIDE C = A.\B, sparse matrix element-wise division.
% C = A.\B is the same as C = B./A.  See rdivide for more details.
%
% See also GrB/rdivide.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = rdivide (B, A) ;

