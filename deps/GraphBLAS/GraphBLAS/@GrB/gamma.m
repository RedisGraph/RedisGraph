function C = gamma (G)
%GAMMA gamma function.
% C = gamma (G) is the gamma function of each entry of G.
% Since gamma (0) = inf, the result is a full matrix.  G must be real.
%
% See also GrB/gammaln.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
type = gbtype (G) ;
if (gb_contains (type, 'complex'))
    error ('input must be real') ;
end
if (~gb_isfloat (type))
    type = 'double' ;
end

C = GrB (gbapply ('gamma', gbfull (G, type))) ;

