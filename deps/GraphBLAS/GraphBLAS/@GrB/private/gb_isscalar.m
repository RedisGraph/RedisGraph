function s = gb_isscalar (G)
%GB_ISSCALAR determine if the GraphBLAS matrix is a scalar.
% isscalar (G) is true for an m-by-n GraphBLAS matrix if m and n are 1.
% G is an opaque GraphBLAS struct or a MATLAB matrix.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

[m, n] = gbsize (G) ;
s = (m == 1) && (n == 1) ;

