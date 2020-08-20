function p = symrcm (G)
%SYMRCM Reverse Cuthill-McKee ordering of a GraphBLAS matrix.
% See 'help symrcm' for details.
%
% See also symrcm, GrB/amd, GrB/colamd, GrB/symamd.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

p = builtin ('symrcm', logical (G)) ;

