function C = fix (G)
%FIX Round towards zero.
% C = fix (G) rounds the entries in the matrix G to the nearest integers
% towards zero.
%
% See also GrB/ceil, GrB/floor, GrB/round.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

Q = G.opaque ;

if (gb_isfloat (gbtype (Q)) && gbnvals (Q) > 0)
    C = GrB (gbapply ('trunc', Q)) ;
else
    C = G ;
end

