function C = floor (G)
%FLOOR round entries to nearest integers towards -infinity.
% C = floor (G) rounds the entries in the matrix G to the nearest integers
% towards -infinity.
%
% See also GrB/ceil, GrB/round, GrB/fix.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

Q = G.opaque ;

if (gb_isfloat (gbtype (Q)) && gbnvals (Q) > 0)
    C = GrB (gbapply ('floor', Q)) ;
else
    C = G ;
end

