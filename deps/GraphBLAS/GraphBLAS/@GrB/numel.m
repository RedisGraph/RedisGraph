function s = numel (G)
%NUMEL the maximum number of entries in a matrix.
% numel (G) is m*n for the m-by-n GraphBLAS matrix G.  If m, n, or m*n
% exceed flintmax (2^53), the result is returned as a vpa symbolic value,
% to avoid integer overflow.
%
% See also GrB/nnz.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
s = gb_numel (G) ;

