function C = real (G)
%REAL complex real part.
% C = real (G) returns the real part of the GraphBLAS matrix G.  Since
% all GraphBLAS matrices are currently real, real (G) is just G.  Complex
% support will be added in the future.
%
% See also GrB/conj.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = G ;

