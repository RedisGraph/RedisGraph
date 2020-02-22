function C = ceil (G)
%CEIL round entries of a GraphBLAS matrix to nearest integers towards inf.
% See also floor, round, fix.

% FUTURE: this will be much faster as a mexFunction.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (isfloat (G) && GrB.entries (G) > 0)
    [m, n] = size (G) ;
    desc.base = 'zero-based' ;
    [i, j, x] = GrB.extracttuples (G, desc) ;
    C = GrB.build (i, j, ceil (x), m, n, desc) ;
else
    C = G ;
end

