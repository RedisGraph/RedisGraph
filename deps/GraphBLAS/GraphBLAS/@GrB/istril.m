function s = istril (G)
%ISTRIL determine if a matrix is lower triangular.
% istril (G) is true if all entries in G are on or below the diagonal.  A
% GraphBLAS matrix G may have explicit zeros.  If these appear in the upper
% triangular part of G, then istril (G) is false, but istril (double (G))
% can be true since double (G) drops those entries.
%
% See also GrB/istriu, GrB/isbanded.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

% FUTURE: this will be much faster when written as a mexFunction
% that doesn't rely on gbselect.  Use a gb_bandwith mexFunction.

G = G.opaque ;
s = (gbnvals (gbselect ('triu', G, 1)) == 0) ;

