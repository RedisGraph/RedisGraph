function C = asin (G)
%ASIN inverse sine.
% C = asin (G) is the inverse sine of each entry of G.
% C is complex if any entry in any (abs(G) > 1).
%
% See also GrB/sin, GrB/sinh, GrB/asinh.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
C = GrB (gb_trig ('asin', G)) ;

