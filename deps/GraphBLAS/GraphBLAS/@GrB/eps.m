function C = eps (G)
%EPS Spacing of floating-point numbers in a GraphBLAS matrix.
%
% See also isfloat, realmax, realmin.

% FUTURE: this will be much faster as a mexFunction.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (~isfloat (G))
    gb_error ('Type must be ''single'', ''double'', or ''complex''') ;
end

[m, n] = size (G) ;
desc.base = 'zero-based' ;
[i, j, x] = GrB.extracttuples (full (G), desc) ;
C = GrB.build (i, j, eps (x), m, n, desc) ;

