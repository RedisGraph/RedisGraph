function p = amd (G, varargin)
%AMD approximate minimum degree ordering of a GraphBLAS matrix.
% See 'help amd' for details.
%
% See also amd, GrB/colamd, GrB/symrcm.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

p = builtin ('amd', logical (G), varargin {:}) ;

