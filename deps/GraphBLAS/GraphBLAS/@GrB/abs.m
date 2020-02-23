function C = abs (G)
%ABS Absolute value of a GraphBLAS matrix.
%
% See also sign.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = GrB.apply ('abs', G) ;

