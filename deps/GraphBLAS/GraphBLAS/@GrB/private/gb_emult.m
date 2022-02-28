function C = gb_emult (A, op, B)
%GB_EMULT C = A.*B, sparse matrix element-wise multiplication.
% C = gb_emult (A, op, B) computes the element-wise multiplication of A
% and B using the operator op, where the op is '*' for C=A.*B.  If both A
% and B are matrices, the pattern of C is the intersection of A and B.  If
% one is a scalar, the pattern of C is the same as the pattern of the one
% matrix.
%
% The input matrices may be either GraphBLAS structs and/or built-in
% matrices, in any combination.  C is returned as a GraphBLAS struct.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

if (gb_isscalar (A) || gb_isscalar (B))
    % either A or B are scalars
    C = gbapply2 (A, op, B) ;
else
    % both A and B are matrices
    C = gbemult (A, op, B) ;
end

