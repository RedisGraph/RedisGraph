function s = gbtest_eq (A, B)
%GBTEST_EQ tests if A and B are equal, after dropping zeros.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

s = isequal (GrB.prune (A), GrB.prune (B)) ;

