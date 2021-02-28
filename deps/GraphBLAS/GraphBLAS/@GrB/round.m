function C = round (G)
%ROUND round entries of a GraphBLAS matrix to the nearest integers
% C = round (G) rounds the entries in the GraphBLAS matrix G to the
% nearest integers.
%
% See also ceil, floor, fix.

% FUTURE: this will be much faster as a mexFunction.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (isfloat (G) && GrB.entries (G) > 0)
    [m, n] = size (G) ;
    desc.base = 'zero-based' ;
    [i, j, x] = GrB.extracttuples (G, desc) ;
    C = GrB.build (i, j, round (x), m, n, desc) ;
else
    C = G ;
end

