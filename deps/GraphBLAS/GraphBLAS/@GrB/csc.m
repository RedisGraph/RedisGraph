function C = csc (G)
%CSC cosecant.
% C = csc (G) is the cosecant of each entry of G.
% Since csc (0) is nonzero, C is a full matrix.
%
% See also GrB/acsc, GrB/csch, GrB/acsch.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
if (~gb_isfloat (gbtype (G)))
    op = 'sin.double' ;
else
    op = 'sin' ;
end

C = GrB (gbapply ('minv', gbfull (gbapply (op, G)))) ;

