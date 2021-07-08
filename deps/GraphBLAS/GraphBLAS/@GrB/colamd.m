function [p, varargout] = colamd (G, varargin)
%COLAMD column approximate minimum degree ordering.
% See 'help colamd' for details.
%
% See also GrB/amd.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

[p, varargout{1:nargout-1}] = colamd (double (G), varargin {:}) ;

