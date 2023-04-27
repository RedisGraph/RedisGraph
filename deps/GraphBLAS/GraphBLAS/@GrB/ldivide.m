function C = ldivide (A, B)
%LDIVIDE C = A.\B, sparse matrix element-wise division.
% C = A.\B is the same as C = B./A.  See rdivide for more details.
%
% See also GrB/rdivide.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

C = rdivide (B, A) ;

