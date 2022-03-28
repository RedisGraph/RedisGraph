function i = end (G, k, ndims)
%END Last index in an indexing expression for a GraphBLAS matrix.
%
% See also GrB/size, GrB/length.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

% FUTURE: add linear indexing
% FUTURE: use hypersparse matrices to implement multidimensionl nD arrays

if (ndims == 1)
    if (~isvector (G))
        error ('Linear indexing not yet supported') ;
    end
    i = length (G) ;
elseif (ndims == 2)
    s = size (G) ;
    i = s (k) ;
else
    error ('%dD indexing not yet supported', ndims) ;
end

