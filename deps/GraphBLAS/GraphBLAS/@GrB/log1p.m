function C = log1p (G)
%LOG1P natural logarithm.
% C = log1p (G) is log(1+x) for each entry x of G.
% If any entry in G is < -1, the result is complex.
%
% See also GrB/log, GrB/log2, GrB/log10, GrB/exp.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
C = GrB (gb_trig ('log1p', G)) ;

