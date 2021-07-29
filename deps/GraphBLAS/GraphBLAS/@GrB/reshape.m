function C = reshape (G, varargin)
%RESHAPE reshape a matrix.
% C = reshape (G, m, n) or C = reshape (G, [m n]) returns the m-by-n
% matrix whose elements are taken columnwise from G.  The matrix G must
% have numel (G) == m*n.  That is numel (G) == numel (C) must be true.
%
% See also GrB/numel, squeeze.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

% FUTURE: this would be faster as a built-in GxB_reshape function.

if (isobject (G))
    G = G.opaque ;
end

[mold, nold, ~] = gbsize (G) ;
mold = int64 (mold) ;
nold = int64 (nold) ;

[mnew, nnew] = gb_parse_dimensions (varargin {:}) ;
mnew = int64 (mnew) ;
nnew = int64 (nnew) ;

if (mold * nold ~= mnew * nnew)
    error ('number of elements must not change') ;
end

desc.base = 'zero-based' ;
[iold, jold, x] = gbextracttuples (G, desc) ;
% convert i and j from 2D (mold-by-nold) to 1D indices
k = gb_convert_index_2d_to_1d (iold, jold, mold) ;
% convert k from 1D indices to 2D (mnew-by-nnew)
[inew, jnew] = gb_convert_index_1d_to_2d (k, mnew) ;
% rebuild the new matrix
C = GrB (gbbuild (inew, jnew, x, mnew, nnew, desc)) ;

