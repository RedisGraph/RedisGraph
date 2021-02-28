function s = norm (G,kind)
%NORM norm of a GraphBLAS sparse matrix.
%
% If G is a matrix:
%
%   norm (G,1) is the maximum sum of the columns of abs (G).
%   norm (G,inf) is the maximum sum of the rows of abs (G).
%   norm (G,'fro') is the Frobenius norm of G: the sqrt of the sum of the
%       squares of the entries in G.
%   The 2-norm is not available for either MATLAB or GraphBLAS sparse
%       matrices.
%
% If G is a row or column vector:
%
%   norm (G,1) is the sum of abs (G)
%   norm (G,2) is the sqrt of the sum of G.^2
%   norm (G,inf) is the maximum of abs (G)
%   norm (G,-inf) is the minimum of abs (G)
%
% See also GrB.reduce, GrB.normdiff.

% FUTURE: the p-norm is not yet supported.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 1)
    kind = 2 ;
end

if (isa (G, 'GrB'))
    G = G.opaque ;
end

s = gbnorm (G, kind) ;

