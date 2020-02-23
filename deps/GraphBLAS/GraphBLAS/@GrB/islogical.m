function s = islogical (G)
%ISLOGICAL true for logical GraphBLAS matrices.
% islogical (G) is true if the GraphBLAS matrix G has the logical type.
%
% See also isnumeric, isfloat, isreal, isinteger, GrB.type, isa, GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

s = isequal (gbtype (G.opaque), 'logical') ;

