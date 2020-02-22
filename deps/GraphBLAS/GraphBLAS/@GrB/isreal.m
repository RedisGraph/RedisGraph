function s = isreal (G)
%ISREAL true for real GraphBLAS matrices.
% isreal (G) is true for a GraphBLAS matrix G, unless it has a type of
% 'complex'.  Note that complex matrices are not yet supported, so
% currently isreal (G) is true for all GraphBLAS matrices.
%
% See also isnumeric, isfloat, isinteger, islogical, GrB.type, isa, GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

s = ~isequal (gbtype (G.opaque), 'complex') ;

