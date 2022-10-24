function k = gb_2d_to_1d (i, j, m)
%GB_2D_TO_1D convert 2D indices to 1D; the indices must be zero-based.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

k = i + j * m ;

