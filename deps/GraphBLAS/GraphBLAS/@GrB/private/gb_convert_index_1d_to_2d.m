function [i, j] = gb_convert_index_1d_to_2d (k, m)
%GB_CONVERT_INDEX_1D_TO_2D convert 1D indices to 2D
% the indices must be zero-based.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

i = rem (k, m) ;
j = (k - i) / m ;

