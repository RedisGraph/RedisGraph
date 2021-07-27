function C = gt (A, B)
%A > B greater than.
% C = (A > B) compares A and B element-by-element.  One or
% both may be scalars.  Otherwise, A and B must have the same size.
%
% See also GrB/lt, GrB/le, GrB/ge, GrB/ne, GrB/eq.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

% FUTURE: gt(A,B) for two matrices A and B is slower than it could be.
% See comments in gb_union_op.

C = lt (B, A) ;

