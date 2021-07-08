function s = issparse (G) %#ok<INUSD>
%ISSPARSE always true for any GraphBLAS matrix.
% A GraphBLAS matrix always keeps track of its pattern, even if all entries
% are present.  Thus, issparse (G) is always true for any GraphBLAS matrix
% G; even issparse (full (G)) is true.
%
% To check if all entries are present in G, use GrB.isfull (G).  The
% expression GrB.isfull (full (G)) is always true.  GrB.isfull (G) is false
% if GrB.entries (G) < prod (size (G)).
%
% See also GrB/ismatrix, GrB/isvector, GrB/isscalar, GrB/isfull, GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

s = true ;

