function s = isdiag (G)
%ISDIAG true if G is a diagonal matrix.
% isdiag (G) is true if G is a diagonal matrix, and false otherwise.
%
% See also GrB/isbanded.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

% FUTURE: this will be faster when 'gb_bandwidth' is a mexFunction,
% but this version is fairly fast anyway.

G = G.opaque ;

% using gb_bandwidth instead:
% [lo, hi] = gb_bandwidth (G) ;
% s = (lo == 0) & (hi == 0) ;

s = (gbnvals (gbselect ('diag', G, 0)) == gbnvals (G)) ;

