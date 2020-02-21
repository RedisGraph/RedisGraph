function I = irand (imin, imax, m, n)
%IRAND construct a random integer matrix 
%
% return a random m-by-n matrix of integers (uint64)
% in the range imin:imax, inclusive
%
% I = irand (imin, imax, m, n)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin < 4)
    n = 1 ;
end

if (nargin < 3)
    m = 1 ;
end

I = uint64 (floor ((imax-imin+1) * rand (m, n)) + imin) ;

assert (min (min (I)) >= imin) ;
assert (max (max (I)) <= imax) ;


