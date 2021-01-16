function [m, n, type] = gb_parse_args (func, varargin)
%GB_PARSE_ARGS parse arguments for true, false, ones, zeros, eye,
% and speye.
%
%   C = ones ;
%   C = ones (n) ;
%   C = ones (m,n) ;
%   C = ones ([m n]) ;
%   C = ones (... , 'like', G) ;
%   C = ones (... , 'int8') ;

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

% parse the type
type = 'double' ;
nargs = length (varargin) ;
for k = 1:nargs
    arg = varargin {k} ;
    if (ischar (arg))
        if (isequal (arg, 'like'))
            if (nargs ~= k+1)
                error ('usage: %s (m, n, ''like'', G)', func) ;
            end
            arg = varargin {k+1} ;
            if (isobject (arg))
                arg = arg.opaque ;
            end
            type = gbtype (arg) ;
        else
            if (nargs ~= k)
                error ('usage: %s (m, n, type)', func) ;
            end
            type = arg ;
        end
        nargs = k-1 ;
        break ;
    end
end

% parse the dimensions
[m, n] = gb_parse_dimensions (varargin {1:nargs}) ;

