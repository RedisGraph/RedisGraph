function s = gb_isfloat (type)
%GB_ISFLOAT true for floating-point GraphBLAS types.
% Implements s = isfloat (type (G))

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

s = gb_contains (type, 'double') || gb_contains (type, 'single') ;

