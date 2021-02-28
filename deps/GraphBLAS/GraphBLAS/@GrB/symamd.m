function [p, varargout] = symamd (G, varargin)
%SYMAMD approximate minimum degree ordering of a GraphBLAS matrix.
% See 'help symamd' for details.
%
% See also symamd, GrB/amd, GrB/colamd, GrB/symrcm.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[p, varargout{1:nargout-1}] = symamd (double (G), varargin {:}) ;

