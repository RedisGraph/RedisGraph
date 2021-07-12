function p = symrcm (G)
%SYMRCM reverse Cuthill-McKee ordering.
% See 'help symrcm' for details.
%
% See also GrB/amd, GrB/colamd, GrB/symamd.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

p = builtin ('symrcm', logical (G)) ;

