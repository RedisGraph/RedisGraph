function C = isfinite (G)
%ISFINITE True for finite elements.
% C = isfinite (G) returns a GraphBLAS logical matrix where C(i,j) = true
% if G(i,j) is finite.
%
% See also isnan, isinf.

% FUTURE: this will be much faster as a mexFunction.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m, n] = size (G) ;
if (isfloat (G) && m > 0 && n > 0)
    desc.base = 'zero-based' ;
    [i, j, x] = GrB.extracttuples (full (G), desc) ;
    C = GrB.build (i, j, isfinite (x), m, n, desc) ;
else
    % C is all true
    C = GrB (true (m, n)) ;
end

