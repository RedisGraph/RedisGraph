function result = gb_printf_helper (printf_function, varargin)
%GB_PRINTF_HELPER wrapper for fprintf and sprintf

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

% convert all GraphBLAS matrices to full MATLAB matrices
len = length (varargin) ;
for k = 2:len
    arg = varargin {k} ;
    if (isobject (arg))
        arg = arg.opaque ;
        desc.kind = 'full' ;
        varargin {k} = gbfull (arg, gbtype (arg), 0, desc) ;    % as MATLAB full
    end
end

% call the built-in fprintf or sprintf
result = builtin (printf_function, varargin {:}) ;

