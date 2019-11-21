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
% See also GrB.reduce.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 1)
    kind = 2 ;
end
if (kind == 0)
    gb_error ('unknown norm') ;
end

if (ischar (kind))
    if (isequal (kind, 'fro'))
        kind = 0 ;
    else
        gb_error ('unknown norm') ;
    end
end

if (isvector (G))
    if (kind == 1)
        s = sum (abs (G)) ;
    elseif (kind == 2 || kind == 0)
        s = sqrt (sum (G.^2)) ;
    elseif (kind == inf)
        s = max (abs (G)) ;
    elseif (kind == -inf)
        s = min (abs (G)) ;
    else
        gb_error ('unknown norm') ;
    end
else
    if (kind == 1)
        s = max (sum (abs (G))) ;
    elseif (kind == 2)
        gb_error ('Sparse norm (G,2) is not available.') ;
    elseif (kind == 0)
        s = sqrt (sum (G.^2, 'all')) ;
    elseif (kind == inf)
        s = max (sum (abs (G), 2)) ;
    elseif (kind == -inf)
        gb_error ('Sparse norm(G,-inf) is not available.') ;
    else
        gb_error ('unknown norm') ;
    end
end

s = full (double (s)) ;

