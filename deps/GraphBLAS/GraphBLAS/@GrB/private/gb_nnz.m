function e = gb_nnz (G)
%GB_NNZ the number of nonzeros in a GraphBLAS matrix.
% Implements e = nnz (G)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

% count entries in G and then subtract the number explicit zero entries
e = gbnvals (G) - gbnvals (gbselect (G, '==0')) ;

