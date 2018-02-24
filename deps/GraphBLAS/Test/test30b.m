function test30b
%TEST30B test GrB_assign

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

%% this test is too slow when debugging
debug = GB_mex_debug ;

if (debug == 0)

    Prob = ssget (2662) ;
    A = Prob.A ;

    [m n] = size (A) ;

    ni =  500 ;
    nj = 1000 ;
    I = randperm (m,ni) ;
    J = randperm (n,nj) ;
    I0 = uint64 (I-1) ;
    J0 = uint64 (J-1) ;

    scalar = sparse (pi) ;

    fprintf ('start GraphBLAS:\n') ;
    tic 
    C2 = GB_mex_assign (A, [], [], scalar, I0, J0, []) ;
    toc

    C = A ; 
    fprintf ('start MATLAB:\n') ;
    tic 
    C (I,J) = scalar ;
    toc

    assert (isequal (C, C2.matrix)) ;
    fprintf ('\ntest30b: all tests passed\n') ;

else
    fprintf ('\ntest30b: tests skipped when NDEBUG enabled\n') ;
end


