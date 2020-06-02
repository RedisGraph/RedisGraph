function C = reshape (G, arg1, arg2)
%RESHAPE Reshape a GraphBLAS matrix.
% C = reshape (G, m, n) or C = reshape (G, [m n]) returns the m-by-n
% matrix whose elements are taken columnwise from G.  The matrix G must
% have numel (G) == m*n.  That is numel (G) == numel (C) must be true.
%
% See also numel, squeeze.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[mold, nold] = size (G) ;
mold = int64 (mold) ;
nold = int64 (nold) ;
if (nargin == 2)
    if (length (arg1) ~= 2)
        gb_error ('reshape (G,s): s must have exactly two elements') ;
    end
    mnew = int64 (arg1 (1)) ;
    nnew = int64 (arg1 (2)) ;
elseif (nargin == 3)
    if (~isscalar (arg1) || ~isscalar (arg2))
        gb_error ('reshape (G,m,n): m and n must be scalars') ;
    end
    mnew = int64 (arg1) ;
    nnew = int64 (arg2) ;
end
if (mold * nold ~= mnew * nnew)
    gb_error ('number of elements must not change') ;
end
if (isempty (G))
    C = GrB (mnew, nnew, GrB.type (G)) ;
else
    desc.base = 'zero-based' ;
    [iold, jold, x] = GrB.extracttuples (G, desc) ;
    % convert i and j from 2D (mold-by-nold) to 1D indices
    k = gb_convert_index_2d_to_1d (iold, jold, mold) ;
    % convert k from 1D indices to 2D (mnew-by-nnew)
    [inew, jnew] = gb_convert_index_1d_to_2d (k, mnew) ;
    % rebuild the new matrix
    C = GrB.build (inew, jnew, x, mnew, nnew, desc) ;
end

