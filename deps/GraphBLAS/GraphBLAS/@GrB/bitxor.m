function C = bitxor (A, B, assumedtype)
%BITXOR bitwise XOR.
% C = bitxor (A,B) is the bitwise XOR of A and B.  If A and B are
% matrices, the pattern of C is the set union of A and B.  If one of A or
% B is a nonzero scalar, the scalar is expanded into a full matrix the
% size of the other matrix, and the result is a full matrix.
%
% With a third parameter, C = bitxor (A,B,assumedtype) provides a data
% type to convert A and B to if they are floating-point types.  If A or B
% already have integer types, then they are not modified.  Otherwise, A or
% B are converted to assumedtype, which can be 'int8', 'int16', 'int32',
% 'int64', 'uint8', 'uint16', 'uint32' or 'uint64'.  The default is
% 'uint64'.
%
% Example:
%
%   A = GrB (magic (4), 'uint8')
%   B = GrB (13 * eye (4), 'uint8') ;
%   B (3,4) = 107
%   C = bitxor (A, B)
%   fprintf ('\nA: ') ; fprintf ('%3x ', A) ; fprintf ('\n') ;
%   fprintf ('\nB: ') ; fprintf ('%3x ', B) ; fprintf ('\n') ;
%   fprintf ('\nC: ') ; fprintf ('%3x ', C) ; fprintf ('\n') ;
%   % in MATLAB:
%   C2 = bitxor (uint8 (A), uint8 (B))
%   isequal (C2, C)
%
% See also GrB/bitor, GrB/bitand, GrB/bitcmp, GrB/bitshift, GrB/bitget,
% GrB/bitset, GrB/bitclr.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

if (nargin < 3)
    assumedtype = 'uint64' ;
end

C = GrB (gb_bitwise ('bitxor', A, B, assumedtype)) ;

