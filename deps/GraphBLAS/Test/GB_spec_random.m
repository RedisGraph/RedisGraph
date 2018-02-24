function A = GB_spec_random (m, n, d, scale, class)
%GB_SPEC_RANDOM generate random matrix
A.matrix = scale * sprandn (m, n, d) ;
A.class = class ;
A.pattern = logical (spones (A.matrix)) ;

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

