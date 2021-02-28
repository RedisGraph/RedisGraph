function count = fprintf (varargin)
%FPRINTF Write formatted data to a text file.
% The GraphBLAS fprintf function is identical to the built-in MATLAB function;
% this overloaded method simply typecasts any GraphBLAS matrices to MATLAB
% matrices first, and then calls the builtin fprintf.
%
% See also fprintf, sprintf, GrB/sprintf.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

c = gb_printf_helper ('fprintf', varargin {:}) ;
if (nargout > 0)
    count = c ;
end

