function p = amd (G, varargin)
%AMD approximate minimum degree ordering.
% See 'help amd' for details.
%
% See also GrB/colamd, GrB/symrcm.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

p = builtin ('amd', logical (G), varargin {:}) ;

