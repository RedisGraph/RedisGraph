function C = cosh (G)
%COSH hyperbolic cosine.
% C = cosh (G) is the hyperbolic cosine of each entry of G.
% Since cosh (0) = 1, the result is a full matrix.
%
% See also GrB/cos, GrB/acos, GrB/acosh.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
type = gbtype (G) ;
if (~gb_isfloat (type))
    type = 'double' ;
end

C = GrB (gbapply ('cosh', gbfull (G, type))) ;

