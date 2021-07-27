function C = asec (G)
%ASEC inverse secant.
% C = asec (G) is the inverse secant of each entry of G.
% Since asec (0) is nonzero, the result is a full matrix.
% C is complex if any (abs(G) < 1).
%
% See also GrB/sec, GrB/sech, GrB/asech.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
type = gbtype (G) ;
if (~gb_isfloat (type))
    type = 'double' ;
end

C = GrB (gb_trig ('acos', gbapply ('minv', gbfull (G, type)))) ;

