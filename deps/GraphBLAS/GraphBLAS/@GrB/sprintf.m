function s = sprintf (varargin)
%SPRINTF write formatted data to a string.
% The GraphBLAS sprintf function is identical to the built-in function;
% this overloaded method simply typecasts any GraphBLAS matrices to
% built-in matrices first, and then calls the builtin sprintf.
%
% See also fprintf, sprintf, GrB/fprintf.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

s = gb_printf_helper ('sprintf', varargin {:}) ;

