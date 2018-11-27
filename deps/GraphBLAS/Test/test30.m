function test30
%TEST30 test GxB_subassign

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
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

    % tic/toc includes the mexFunction overhead of making a deep copy
    % of the input matrix.  MATLAB can modify C in place, as can GraphBLAS,
    % but GraphBLAS cannot safely do that through a mexFunction interface
    % to MATLAB.

    fprintf ('start GraphBLAS:\n') ;
    tic 
    C2 = GB_mex_subassign (A, [], [], scalar, I0, J0, []) ;
    toc
    t = gbresults

    C = A ; 
    fprintf ('start MATLAB:\n') ;
    tic 
    C (I,J) = scalar ;
    tm = toc

    fprintf ('GraphBLAS speedup over MATLAB: %g\n',  tm/t) ;

    assert (isequal (C, C2.matrix)) ;
    fprintf ('\ntest30: all tests passed\n') ;

else
    fprintf ('\ntest30: tests skipped when NDEBUG enabled\n') ;
end

