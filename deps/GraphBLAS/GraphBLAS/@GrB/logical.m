function C = logical (G)
%LOGICAL typecast a GraphBLAS sparse matrix to MATLAB sparse logical matrix.
% C = logical (G) typecasts the GraphBLAS matrix G to into a MATLAB
% sparse logical matrix.
%
% To typecast the matrix G to a GraphBLAS sparse logical matrix instead,
% use C = GrB (G, 'logical').
%
% See also cast, GrB, double, complex, single, int8, int16, int32, int64,
% uint8, uint16, uint32, and uint64.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = gbsparse (G.opaque, 'logical') ;

