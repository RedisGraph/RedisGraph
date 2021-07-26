function C = erf (G)
%ERF error function.
% C = erf (G) computes the error function of each entry of G.
% G must be real.
%
% See also GrB/erfc, erfcx, erfinv, erfcinv.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
type = gbtype (G) ;
if (gb_contains (type, 'complex'))
    error ('input must be real') ;
end
if (~gb_isfloat (type))
    op = 'erf.double' ;
else
    op = 'erf' ;
end

C = GrB (gbapply (op, G)) ;

