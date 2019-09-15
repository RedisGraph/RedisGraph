%TESTCOV run all GraphBLAS tests, with statement coverage

%  SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
%  http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

all_tcov_time = tic ;
try
    addpath ('../Test') ;
    addpath ('../Test/spok') ;
    addpath ('../Demo/MATLAB') ;
    cd ../Test/spok
    spok_install ;
    cd ../../Tcov
    debug_on ;
    gbcover ;
    testall ;
catch me
    debug_off ;
    rethrow (me) ;
end

t = toc (all_tcov_time) ;
fprintf ('\ntestcov: all tests passed, total time %0.5g minutes\n', t / 60) ;

