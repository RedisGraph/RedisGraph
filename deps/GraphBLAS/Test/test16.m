function test16
%TEST16 test user-defined complex type (runs all testc*.m)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

% all complex matrix tests

for k = [false true]
    GB_builtin_complex_set (k) ;
    if (k)
        fprintf ('\nTesting GxB_FC64 complex type:\n') ;
    else
        fprintf ('\nTesting user-defined Complex type:\n') ;
    end

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
end

GB_builtin_complex_set (true) ;
fprintf ('\ntest16: all complex tests passed\n') ;

