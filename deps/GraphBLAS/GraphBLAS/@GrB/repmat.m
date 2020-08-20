function C = repmat (G, m, n)
%REPMAT Replicate and tile a GraphBLAS matrix.
% C = repmat (G, m, n)  % constructs an m-by-n tiling of the GrB matrix A
% C = repmat (G, [m n]) % same as C = repmat (A, m, n)
% C = repmat (G, n)     % constructs an n-by-n tiling of the GrB matrix G
%
% See also kron, GrB.kronecker.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 3)
    R = ones (m, n, 'logical') ;
else
    R = ones (m, 'logical') ;
end
op = ['2nd.' GrB.type(G)] ;
C = GrB.kronecker (R, op, G) ;

