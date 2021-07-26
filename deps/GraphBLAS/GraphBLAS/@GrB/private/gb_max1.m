function C = gb_max1 (op, A)
%GB_MAX1 single-input max
% Implements C = max (A)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

[m, n] = gbsize (A) ;
if (m == 1 || n == 1)
    % C = max (A) for a vector A results in a scalar C
    C = gb_maxall (op, A) ;
else
    C = gb_maxbycol (op, A) ;
end

