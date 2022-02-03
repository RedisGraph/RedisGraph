function [arg1, arg2] = bandwidth (G, uplo)
%BANDWIDTH matrix bandwidth.
% [lo, hi] = bandwidth (G) returns the upper and lower bandwidth of G.
% lo = bandwidth (G, 'lower') returns just the lower bandwidth.
% hi = bandwidth (G, 'upper') returns just the upper bandwidth.
%
% See also GrB/isbanded, GrB/isdiag, GrB/istril, GrB/istriu.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

% FUTURE: this will be much faster when implemented in a mexFunction.
% It is currently much slower than the built-in bandwidth function.

% compute the bandwidth
G = G.opaque ;
[lo, hi] = gb_bandwidth (G) ;

% return the result
if (nargin == 1)
   arg1 = lo ;
   arg2 = hi ;
else
    if (nargout > 1)
        error ('too many output arguments') ;
    elseif isequal (uplo, 'lower')
        arg1 = lo ;
    elseif isequal (uplo, 'upper')
        arg1 = hi ;
    else
        error ('unrecognized option') ;
    end
end

