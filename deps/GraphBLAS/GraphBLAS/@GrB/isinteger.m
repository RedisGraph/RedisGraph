function s = isinteger (G)
%ISINTEGER true for integer GraphBLAS matrices.
% isinteger (G) is true if the GraphBLAS matrix G has an integer type
% (int8, int16, int32, int64, uint8, uint16, uint32, or uint64).
%
% See also isnumeric, isfloat, isreal, islogical, GrB.type, isa, GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

t = gbtype (G.opaque) ;
s = isequal (t, 'int8'  ) || isequal (t, 'int16' ) || ...
    isequal (t, 'int32' ) || isequal (t, 'int64' ) || ...
    isequal (t, 'uint8' ) || isequal (t, 'uint16') || ...
    isequal (t, 'uint32') || isequal (t, 'uint64') ;

