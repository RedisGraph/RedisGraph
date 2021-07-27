function C = acosh (G)
%ACOSH inverse hyperbolic cosine.
% C = acosh (G) is the inverse hyperbolic cosine of each entry G.
% Since acosh (0) is nonzero, the result is a full matrix.
% C is complex if any (G < 1).
%
% See also GrB/cos, GrB/acos, GrB/cosh.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
C = GrB (gb_trig ('acosh', gbfull (G))) ;

