function i = end (G, k, ndims)
%END Last index in an indexing expression for a GraphBLAS matrix.
%
% See also size, length.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% FUTURE: add linear indexing
% FUTURE: use hypersparse matrices to implement multidimensionl nD arrays

if (ndims == 1)
    if (~isvector (G))
        error ('GrB:unsupported', 'Linear indexing not supported') ;
    end
    i = length (G) ;
elseif (ndims == 2)
    s = size (G) ;
    i = s (k) ;
else
    error ('GrB:unsupported', '%dD indexing not supported', ndims) ;
end

