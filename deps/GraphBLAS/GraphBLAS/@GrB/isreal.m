function s = isreal (G)
%ISREAL true for real matrices.
% isreal (G) is true for a GraphBLAS matrix G, unless it has a type of
% 'single complex' or 'double complex'.
%
% See also GrB/isnumeric, GrB/isfloat, GrB/isinteger, GrB/islogical,
% GrB.type, GrB/isa, GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
s = ~gb_contains (gbtype (G), 'complex') ;

