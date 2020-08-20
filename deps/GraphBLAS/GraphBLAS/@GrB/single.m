function C = single (G)
%SINGLE cast a GraphBLAS matrix to MATLAB full single matrix.
% C = single (G) typecasts the GrB matrix G to a MATLAB full single
% matrix.  The result C is full since MATLAB does not support sparse
% single matrices.
%
% To typecast the matrix G to a GraphBLAS sparse single matrix instead,
% use C = GrB (G, 'single').
%
% See also GrB, double, complex, logical, int8, int16, int32, int64,
% uint8, uint16, uint32, and uint64.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = gbfull (G.opaque, 'single') ;

