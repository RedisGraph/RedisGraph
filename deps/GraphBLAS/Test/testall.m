function testall (longtests)
%TESTALL run all GraphBLAS tests
%
% Usage:
% testall ;         % runs just the shorter tests (about 15 minutes)
% testall(1) ;      % runs all the tests (overnight).  Requires SuiteSparse.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

t = cputime ;

if (nargin < 1)
    % run the shorter tests by default
    longtests = 0 ;
end

% clear the statement coverage counts
clear global GraphBLAS_gbcov

% many of the tests use SuiteSparse/MATLAB_Tools/spok, a copy of which is
% included here in GraphBLAS/Test/spok.
addpath ('../Test/spok') ;
addpath ('../Demo/MATLAB') ;

try
    spok (sparse (1)) ;
catch
    cd spok ; spok_install ; cd ..
end

logstat ;             % start the log.txt

%-------------------------------------------------------------------------------
% quick tests for statement coverage

logstat ('test98') ;  % GB_mex_mxm, typecast on the fly
logstat ('test97') ;  % GB_mex_assign, scalar expansion and zombies
logstat ('test01') ;  % error handling
logstat ('test02') ;  % matrix copy and dup tests
logstat ('test03') ;  % random matrix tests
logstat ('test04') ;  % simple mask and transpose test
logstat ('test05') ;  % quick setElement test, with typecasting
logstat ('test07') ;  % quick test GB_mex_subassign
logstat ('test07b') ; % quick test GB_mex_assign
logstat ('test08') ;  % quick test GB_mex_subassign
logstat ('test09') ;  % duplicate I,J test of GB_mex_subassign
logstat ('test13') ;  % simple tests of GB_mex_transpose
logstat ('test15') ;  % simple test of GB_mex_AxB
logstat ('test17') ;  % quick test of GrB_*_extractElement
logstat ('test72') ;  % several special cases
logstat ('test26') ;  % quick test of GxB_select
logstat ('test29') ;  % reduce with zombies
logstat ('test69') ;  % assign and subassign with alias
logstat ('test28') ;  % mxm with aliased inputs, C<C> = accum(C,C*C)
logstat ('test11') ;  % exhaustive test of GrB_extractTuples
logstat ('test14') ;  % GrB_reduce
logstat ('test20') ;  % quick test of GB_mex_mxm on a few semirings
logstat ('test00') ;  % GB_mex_mis
logstat ('test19') ;  % GxB_subassign, many pending operators
logstat ('test12') ;  % Wathen finite-element matrices (short test)
logstat ('test10') ;  % GrB_apply
logstat ('test76') ;  % GxB_resize
logstat ('test27') ;  % quick test of GxB_select (band)
logstat ('test25') ;  % quick test of GxB_select
logstat ('test74') ;  % test GrB_mxm on all semirings, just dot product method
logstat ('test99') ;  % GB_mex_transpose with explicit zeros in the Mask
logstat ('test23') ;  % quick test of GB_*_build
logstat ('test18') ;  % quick tests of GrB_eWiseAdd and eWiseMult
logstat ('test77') ;  % quick tests of GxB_kron
logstat ('test16') ;  % user-defined complex operators
logstat ('test24') ;  % test of GrB_Matrix_reduce
logstat ('test21') ;  % quick test of GB_mex_subassign
logstat ('test06') ;  % test GrB_mxm on all semirings
logstat ('test75') ;  % test GrB_mxm A'*B on all semirings
logstat ('test19b') ; % GrB_assign, many pending operators
logstat ('test22') ;  % quick test of GB_mex_transpose

%-------------------------------------------------------------------------------
% The following tests are not required for statement coverage.  Some need
% other packages in SuiteSparse (CSparse, SSMULT, ssget).  By default, these
% tests are not run.

if (longtests)
    % useful tests but not needed for statement coverage
    logstat ('test26(1)') ;  % longer test of GxB_select
    logstat ('test20(1)') ;  % test of GB_mex_mxm on all built-in semirings
    logstat ('test18(1)') ;  % lengthy tests of GrB_eWiseAdd and eWiseMult
    logstat ('test08b') ; % quick test GB_mex_assign
    logstat ('test09b') ; % duplicate I,J test of GB_mex_assign
    logstat ('test21b') ; % exhaustive test of GB_mex_assign
    logstat ('test21(1)') ;  % exhaustive test of GB_mex_subassign
    logstat ('test23(1)') ;  % exhaustive test of GB_*_build
    logstat ('test24(1)') ;  % exhaustive test of GrB_Matrix_reduce
    logstat ('test64') ;  % quick test of GB_mex_subassign, scalar expansion
    logstat ('test65') ;  % type casting
    logstat ('test66') ;  % quick test for GrB_Matrix_reduce
    logstat ('test67') ;  % quick test for GrB_apply
    logstat ('test30') ;  % performance test GB_mex_subassign, scalar expansion
    logstat ('test30b') ; % performance test GB_mex_assign, scalar expansion
    logstat ('test31') ;  % simple tests of GB_mex_transpose
    logstat ('test12(0)') ; % Wathen finite-element matrices (full test)
    logstat ('test58(0)') ; % longer GB_mex_eWiseAdd_Matrix performance test
    logstat ('test32') ;  % quick GB_mex_mxm test
    logstat ('test33') ;  % create a semiring
    logstat ('test34') ;  % quick GB_mex_eWiseAdd_Matrix test
    logstat ('test35') ;  % performance test for GrB_extractTuples
    logstat ('test36') ;  % performance test for GB_mex_Matrix_subref
    logstat ('test37') ;  % performance test for GrB_qsort1
    logstat ('test38') ;  % GB_mex_transpose with matrix collection
    logstat ('test39') ;  % tests of GrB_transpose, GB_*_add and eWiseAdd
    logstat ('test40') ;  % test for GrB_Matrix_extractElement, and Vector
    logstat ('test41') ;  % test of GB_mex_AxB and GB_mex_AxB_symbolic
    logstat ('test42') ;  % performance tests for GB_mex_Matrix_build
    logstat ('test43') ;  % performance tests for GB_mex_Matrix_subref
    logstat ('test44') ;  % test qsort
    logstat ('test62') ;  % exhaustive test of GrB_apply
    logstat ('test63') ;  % GB_mex_op and operator tests
    logstat ('test45') ;  % test GB_mex_setElement and build
    logstat ('test46') ;  % performance test GB_mex_subassign
    logstat ('test46b') ; % performance test GB_mex_assign
    logstat ('test47') ;
    logstat ('test48') ;
    logstat ('test49') ;
    logstat ('test50') ;  % test GB_mex_AxB on larger matrix
    logstat ('test51') ;  % performance test GB_mex_subassign, multiple ops
    logstat ('test51b') ; % performance test GB_mex_assign, multiple ops
    logstat ('test52') ;  % performance of A*B with tall matrices, AdotB, AxB
    logstat ('test53') ;  % exhaustive test of GB_mex_Matrix_extract
    logstat ('test06(936)') ; % performance test of GrB_mxm on all semirings
    logstat ('test54') ;
    logstat ('test55') ;
    logstat ('test55b') ;
    logstat ('test56') ;
    logstat ('test57') ;
    logstat ('test58') ;
    logstat ('test59') ;
    logstat ('test60') ;
    logstat ('test61') ;
end

fprintf ('\ntestall: all tests passed, total time %g sec\n', cputime-t) ;

