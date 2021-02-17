function C = uint64 (G)
%UINT64 cast a GraphBLAS matrix to MATLAB full uint64 matrix.
% C = uint64 (G) typecasts the GrB matrix G to a MATLAB full uint64
% matrix.  The result C is full since MATLAB does not support sparse
% uint64 matrices.
%
% To typecast the matrix G to a GraphBLAS sparse uint64 matrix instead,
% use C = GrB (G, 'uint64').
%
% See also GrB, GrB/double, GrB/complex, GrB/single, GrB/logical, GrB/int8,
% GrB/int16, GrB/int32, GrB/int64, GrB/uint8, GrB/uint16, GrB/uint32.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

G = G.opaque ;
desc.kind = 'full' ;
C = gbfull (G, 'uint64', uint64 (0), desc) ;    % export as a MATLAB full matrix

