function C = repmat (G, m, n)
%REPMAT replicate and tile a matrix.
% C = repmat (G, m, n)  % constructs an m-by-n tiling of the matrix G
% C = repmat (G, [m n]) % same as C = repmat (A, m, n)
% C = repmat (G, n)     % constructs an n-by-n tiling of the matrix G
%
% See also GrB/kron, GrB.kronecker.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
type = gbtype (G) ;

if (nargin == 3)
    R = ones (m, n, 'logical') ;
else
    R = ones (m, 'logical') ;
end
op = ['2nd.' type] ;
C = GrB (gbkronecker (R, op, G)) ;

