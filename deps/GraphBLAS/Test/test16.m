function test16
%TEST16 test user-defined complex type (runs all testc*.m)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% all complex matrix tests

fprintf ('\nTesting user-defined complex type:\n') ;
testc1  % test ops
testc2  % A'*B, A+B, A*B
testc3  % extract column, extract submatrix
testc4  % extractElement, setElement
testc5  % subref
testc6  % apply
testc7  % assign
testc8  % eWise
testc9  % extractTuples
testca  % mxm, mxv, vxm
testcb  % reduce
testcc  % transpose

fprintf ('\ntest16: all complex tests passed\n') ;

