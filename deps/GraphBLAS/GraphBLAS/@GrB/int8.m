function C = int8 (G)
%INT8 cast a GraphBLAS matrix to MATLAB full int8 matrix.
% C = int8 (G) typecasts the GrB matrix G to a MATLAB full int8 matrix.
% The result C is full since MATLAB does not support sparse int8
% matrices.
%
% To typecast the matrix G to a GraphBLAS sparse int8 matrix instead, use
% C = GrB (G, 'int8').
%
% See also GrB, GrB/double, GrB/complex, GrB/single, GrB/logical,
% GrB/int16, GrB/int32, GrB/int64, GrB/uint8, GrB/uint16, GrB/uint32,
% GrB/uint64.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

G = G.opaque ;
desc.kind = 'full' ;
C = gbfull (G, 'int8', int8 (0), desc) ;        % export as a MATLAB full matrix

