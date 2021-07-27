function s = isfloat (G)
%ISFLOAT true for floating-point matrices.
% isfloat (G) is true if the matrix G has a type of 'double',
% 'single', 'single complex', or 'double complex'.
%
% See also GrB/isnumeric, GrB/isreal, GrB/isinteger, GrB/islogical,
% GrB.type, GrB/isa, GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
s = gb_isfloat (gbtype (G)) ;

