function count = fprintf (varargin)
%FPRINTF Write formatted data to a text file.
% The GraphBLAS fprintf function is identical to the built-in
% function; this overloaded method simply typecasts any GraphBLAS matrices
% to built-in matrices first, and then calls the builtin fprintf.
%
% See also fprintf, sprintf, GrB/sprintf.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

c = gb_printf_helper ('fprintf', varargin {:}) ;
if (nargout > 0)
    count = c ;
end

