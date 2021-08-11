function [m, n] = size (G, dim)
%SIZE the dimensions of a matrix.
% [m n] = size (G) is the size of an m-by-n sparse matrix.
% If any dimension exceeds flintmax (2^53), m and n are returned as int64.
%
% See also GrB/length, GrB/numel.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
[m, n] = gbsize (G) ;

if (nargin == 2)
    s = [m n] ;
    m = s (dim) ;
elseif (nargout == 1)
    m = [m n] ;
end

