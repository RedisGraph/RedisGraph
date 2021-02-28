function s = sprintf (varargin)
%SPRINTF Write formatted data to a string.
% The GraphBLAS sprintf function is identical to the built-in MATLAB
% function; this overloaded method simply typecasts any GraphBLAS
% matrices to MATLAB matrices first, and then calls the builtin sprintf.
%
% See also fprintf, sprintf, GrB/fprintf.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

s = gb_printf_helper ('sprintf', varargin {:}) ;

