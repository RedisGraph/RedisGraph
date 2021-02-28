function test33
%TEST33 test a semiring

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

semiring = struct ( ...
    'multiply', 'times', ...
    'add', 'plus', ...
    'class', 'double' )

GB_mex_semiring (semiring)

fprintf ('\ntest33: all tests passed\n') ;

