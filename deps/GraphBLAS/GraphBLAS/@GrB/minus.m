function C = minus (A, B)
%MINUS sparse matrix subtraction, C = A-B.
% C = A-B subtracts the two matrices A and B.  If A and B are matrices,
% the pattern of C is the set union of A and B.  If one of A or B is a
% scalar, the scalar is expanded into a full matrix the size of the
% other matrix, and the result is a full matrix.
%
% See also GrB.eadd, GrB/plus, GrB/uminus.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

% FUTURE: minus(A,B) for two matrices A and B is slower than it could be.
% See comments in gb_union_op.

if (isobject (A))
    A = A.opaque ;
end

if (isobject (B))
    B = B.opaque ;
end

type = gboptype (gbtype (A), gbtype (B)) ;
C = GrB (gb_eadd (A, '+', gbapply (['-.' type], B))) ;

