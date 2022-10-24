function C = ctranspose (G)
%CTRANSPOSE C = G', transpose a GraphBLAS matrix.
% C = G' is the complex conjugate transpose of G.
%
% See also GrB.trans, GrB/transpose, GrB/conj.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;

if (gb_contains (gbtype (G), 'complex'))
    desc.in0 = 'transpose' ;
    C = GrB (gbapply ('conj', G, desc)) ;
else
    C = GrB (gbtrans (G)) ;
end

