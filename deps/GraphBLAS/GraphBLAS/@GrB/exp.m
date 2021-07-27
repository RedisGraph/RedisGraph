function C = exp (G)
%EXP exponential.
% C = exp (G) is e^x for each entry x of the matrix G.
% Since e^0 is nonzero, C is a full matrix.
%
% See also GrB/exp, GrB/expm1, GrB/pow2, GrB/log, GrB/log10, GrB/log2.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
type = gbtype (G) ;

if (~gb_isfloat (type))
    type = 'double' ;
end

C = GrB (gbapply ('exp', gbfull (G, type))) ;

