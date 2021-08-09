function C = expm1 (G)
%EXPM1 exp(x)-1.
% C = expm1 (G) computes (e^x)-1 for each entry x of a matrix G.
%
% See also GrB/exp, GrB/expm1, GrB/log, GrB/log10, GrB/log2.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;

if (~gb_isfloat (gbtype (G)))
    op = 'expm1.double' ;
else
    op = 'expm1' ;
end

C = GrB (gbapply (op, G)) ;

