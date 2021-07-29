function C = coth (G)
%COTH hyperbolic cotangent.
% C = coth (G) is the hyperbolic cotangent of each entry of G.
% Since coth (0) is nonzero, C is a full matrix.
%
% See also GrB/cot, GrB/acot, GrB/acoth.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
if (~gb_isfloat (gbtype (G)))
    op = 'tanh.double' ;
else
    op = 'tanh' ;
end

C = GrB (gbapply ('minv', gbfull (gbapply (op, G)))) ;

