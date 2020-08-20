function C = gt (A, B)
%A > B Greater than.
% C = (A > B) is an element-by-element comparison of A and B.  One or
% both may be scalars.  Otherwise, A and B must have the same size.
%
% The input matrices may be either GraphBLAS and/or MATLAB matrices, in
% any combination.  C is returned as a GraphBLAS matrix.
%
% See also GrB/lt, GrB/le, GrB/ge, GrB/ne, GrB/eq.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = lt (B, A) ;

