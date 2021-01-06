%TESTCOV run all GraphBLAS tests, with statement coverage

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

if (ispc)
    error ('The tests in Tcov are not ported to Windows') ;
end

try
    addpath ('../Test') ;
    addpath ('../Test/spok') ;
    addpath ('../Demo/MATLAB') ;
    cd ../Test/spok
    spok_install ;
    cd ../../Tcov
    mex -O -R2018a ../Test/GB_spones_mex.c
    debug_on ;
    grbcover ;
    testall ;
catch me
    debug_off ;
    rethrow (me) ;
end

fprintf ('\ntestcov: all tests passed\n') ;

