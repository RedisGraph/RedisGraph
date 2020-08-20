function C = double (G)
%DOUBLE cast a GraphBLAS sparse matrix to a MATLAB sparse double matrix.
% C = double (G) typecasts the GraphBLAS matrix G into a MATLAB sparse
% double matrix C, either real or complex.
%
% To typecast the matrix G to a GraphBLAS sparse double (real) matrix
% instead, use C = GrB (G, 'double').
%
% See also cast, GrB, complex, single, logical, int8, int16, int32, int64,
% uint8, uint16, uint32, and uint64.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% if (isreal (G))
    C = gbsparse (G.opaque, 'double') ;
% else
%     FUTURE: complex support
%     C = gbsparse (G.opaque, 'complex') ;
% end

