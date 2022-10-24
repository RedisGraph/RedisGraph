function C = conj (G)
%CONJ complex conjugate.
% C = conj (G) is the complex conjugate of each entry of G.
%
% See also GrB/real, GrB/imag.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

Q = G.opaque ;

if (gb_contains (gbtype (Q), 'complex'))
    C = GrB (gbapply ('conj', Q)) ;
else
    C = G ;
end

