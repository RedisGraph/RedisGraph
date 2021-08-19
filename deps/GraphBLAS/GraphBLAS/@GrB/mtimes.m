function C = mtimes (A, B)
%MTIMES sparse matrix-matrix multiplication over the standard semiring.
% C=A*B multiples two matrices using the standard '+.*' semiring.  If
% either A or B are scalars, C=A*B is the same as C=A.*B.
%
% See also GrB.mxm, GrB.emult, GrB/times.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

if (isobject (A))
    A = A.opaque ;
end

if (isobject (B))
    B = B.opaque ;
end

if (gb_isscalar (A) || gb_isscalar (B))
    C = GrB (gb_emult (A, '*', B)) ;
else
    C = GrB (gbmxm (A, '+.*', B)) ;
end

