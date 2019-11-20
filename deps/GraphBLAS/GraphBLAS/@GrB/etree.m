function [parent, varargout] = etree (G, varargin)
%ETREE Elimination tree of a GraphBLAS matrix.
% See 'help etree' for details.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[parent, varargout{1:nargout-1}] = builtin ('etree', logical (G), varargin {:});

