function C = complex (A, B) %#ok<STOUT,INUSD>
%COMPLEX cast a GraphBLAS matrix to MATLAB sparse double complex matrix.
% C = complex (G) will typecast the GraphBLAS matrix G to into a MATLAB
% sparse complex matrix (complex types are not yet supported, however).
%
% To typecast the matrix G to a GraphBLAS sparse complex matrix instead,
% use C = GrB (G, 'complex').
%
% See also cast, GrB, double, single, logical, int8, int16, int32, int64,
% uint8, uint16, uint32, and uint64.

% FUTURE: add complex type(s)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

error ('GrB:unsupported', 'complex type not supported') ;

