function C = asinh (G)
%ASINH inverse hyperbolic sine.
% C = asinh (G) is the inverse hyberbolic sine of each entry G.
%
% See also GrB/sin, GrB/asin, GrB/sinh.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;

if (~gb_isfloat (gbtype (G)))
    op = 'asinh.double' ;
else
    op = 'asinh' ;
end

C = GrB (gbapply (op, G)) ;

