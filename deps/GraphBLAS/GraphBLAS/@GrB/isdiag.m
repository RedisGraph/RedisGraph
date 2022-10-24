function s = isdiag (G)
%ISDIAG true if G is a diagonal matrix.
% isdiag (G) is true if G is a diagonal matrix, and false otherwise.
%
% See also GrB/isbanded.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

% FUTURE: this will be much faster when written as a mexFunction
% that doesn't rely on gbselect.  Use a gb_bandwith mexFunction.

G = G.opaque ;

s = (gbnvals (gbselect ('diag', G, 0)) == gbnvals (G)) ;

