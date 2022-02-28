function [p, varargout] = symamd (G, varargin)
%SYMAMD approximate minimum degree ordering.
% See 'help symamd' for details.
%
% See also GrB/amd, GrB/colamd, GrB/symrcm.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

[p, varargout{1:nargout-1}] = symamd (double (G), varargin {:}) ;

