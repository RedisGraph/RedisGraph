function [lo, hi] = gb_bandwidth (G)
%GB_BANDWIDTH Determine the bandwidth of a GraphBLAS matrix.
% Implements [lo, hi] = bandwidth (G).

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

% compute the bandwidth
if (gbnvals (G) == 0)
    % matrix is empty
    hi = 0 ;
    lo = 0 ;
else
    desc.base = 'zero-based' ;
    [i, j] = gbextracttuples (G, desc) ;
    b = j - i ;
    hi = max (0,  double (max (b))) ;
    lo = max (0, -double (min (b))) ;
end

