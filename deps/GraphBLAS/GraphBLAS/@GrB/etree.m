function [parent, varargout] = etree (G, varargin)
%ETREE elimination tree of a GraphBLAS matrix.
% See 'help etree' for details.
%
% See also GrB/amd.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = logical (G) ;
[parent, varargout{1:nargout-1}] = builtin ('etree', G, varargin {:}) ;

