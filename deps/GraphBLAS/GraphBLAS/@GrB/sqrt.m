function C = sqrt (G)
%SQRT Square root.
% C = sqrt (G) is the square root of the elements of the GraphBLAS matrix
% G.  Complex matrices are not yet supported, and thus currently all
% entries in G must be nonnegative.
%
% See also GrB.apply.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = G.^(.5) ;

