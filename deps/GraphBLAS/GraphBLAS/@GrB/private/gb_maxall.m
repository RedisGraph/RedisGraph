function C = gb_maxall (op, A)
%GB_MAXALL reduce a matrix to a scalar
% Implements C = max (A, [ ], 'all') ;

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

C = gbreduce (op, A) ;
[m, n] = gbsize (A) ;
if ((m*n ~= gbnvals (A)) && gb_scalar (C) <= 0)
    % A is not full, and the max of the entries present is <= 0,
    % so C is an empty scalar (an implicit zero)
    C = gbnew (1, 1, gbtype (C)) ;
end

