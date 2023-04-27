function C = erfc (G)
%ERFC complementary error function.
% C = erfc (G) is the complementary error function of each entry of G.
% Since erfc (0) = 1, the result is a full matrix.  G must be real.
%
% See also GrB/erf, erfcx, erfinv, erfcinv.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
type = gbtype (G) ;
if (gb_contains (type, 'complex'))
    error ('input must be real') ;
end
if (~gb_isfloat (type))
    type = 'double' ;
end

C = GrB (gbapply ('erfc', gbfull (G, type))) ;

