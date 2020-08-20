function C = isinf (G)
%ISINF True for infinite elements.
% C = isinf (G) returns a GraphBLAS logical matrix where C(i,j) = true
% if G(i,j) is infinite.
%
% See also isnan, isfinite.

% FUTURE: this will be much faster as a mexFunction.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m, n] = size (G) ;
if (isfloat (G) && GrB.entries (G) > 0)
    desc.base = 'zero-based' ;
    [i, j, x] = GrB.extracttuples (G, desc) ;
    C = GrB.build (i, j, isinf (x), m, n, desc) ;
else
    % C is all false
    C = GrB (m, n, 'logical') ;
end

