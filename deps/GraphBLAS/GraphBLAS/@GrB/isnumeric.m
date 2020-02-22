function s = isnumeric (G) %#ok<INUSD>
%ISNUMERIC always true for any GraphBLAS matrix.
% isnumeric (G) is always true for any GraphBLAS matrix G, including
% logical matrices, since those matrices can be operated on in any
% semiring, just like any other GraphBLAS matrix.
%
% See also isfloat, isreal, isinteger, islogical, GrB.type, isa, GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

s = true ;

