function L = laplacian (A, type, check)
%GRB.LAPLACIAN Graph Laplacian matrix
% L = laplacian (A) is the graph Laplacian of the matrix A.  spones(A) must be
% symmetric with no diagonal entries. The diagonal of L is the degree of the
% nodes.  That is, L(j,j) = sum (spones (A (:,j))).  For off-diagonal entries,
% L(i,j) = L(j,i) = -1 if the edge (i,j) exists in A.
%
% The type of L defaults to double.  With a second argument, the type of L can
% be specified, as L = laplacian (A,type); type may be 'double', 'single',
% 'int8', 'int16', 'int32', or 'int64'.  Be aware that integer overflow may
% occur with the smaller integer types.
%
% To check the input matrix, use GrB.laplacian (A, 'double', 'check') ;
%
% L is returned as symmetric matrix.
%
% Example:
%
%   A = bucky ;
%   L = GrB.laplacian (A)
%
% See also graph/laplacian.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin < 2)
    type = 'double' ;
end
if (nargin < 3)
    check = false ;
else
    check = isequal (check, 'check') ;
end

if (~GrB.issigned (type))
    % type must be signed
    gb_error ('invalid type') ;
end

A = GrB.apply (['1.' type], A) ;

if (check)
    if (~issymmetric (A))
        gb_error ('A must be symmetric') ;
    end
    if (GrB.entries (diag (A)) > 0)
        gb_error ('A must have no diagonal entries') ;
    end
end

if (GrB.isbycol (A))
    D = GrB.vreduce ('+', A, struct ('in0', 'transpose')) ;
else
    D = GrB.vreduce ('+', A) ;
end

% construct the Laplacian
L = - GrB.offdiag (A) + diag (D) ;

