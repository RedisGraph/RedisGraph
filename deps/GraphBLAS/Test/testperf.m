function testperf
%TESTPERF run all performance tests

t = tic ;
fprintf ('\ntestperf:  run all performance tests\n') ;

test_other ;    % install required packages

debug_off

test26(1) ;     % performance test for GxB_select
test36 ;        % performance test of matrix subref
test30 ;        % performance test GB_mex_subassign, scalar expansion
test30b ;       % performance test GB_mex_assign, scalar expansionb
test35 ;        % performance test for GrB_extractTuples
test39 ;        % performance test for GrB_transpose
test42 ;        % performance tests for GB_mex_Matrix_build
test43 ;        % performance tests for GB_mex_Matrix_subref
test46 ;        % performance test GB_mex_subassign
test48 ;        % performance test of GrB_mxm
test46b ;       % performance test GB_mex_assign
test49 ;        % performance test of GrB_mxm (dot product method, A'*B)
test51 ;        % performance test GB_mex_subassign, multiple ops
test51b ;       % performance test GB_mex_assign, multiple ops
test58(0)       % longer GB_mex_eWiseAdd_Matrix performance test
test61 ;        % performance test of GrB_eMult
test68 ;        % performance tests for eWiseMult
f = [936 2662] ;
test70 (f) ;    % performance comparison of triangle counting methods
test71 (f) ;    % performance comparison of triangle counting methods
test73 ;        % performance of C = A*B, with mask
test86 ;        % performance of GrB_Matrix_extract
test52 ;        % performance of A*B with tall matrices, AdotB, AxB

% performance test of GrB_mxm on all semirings (just auto method)
test06(936, [ ], 1, 0) ;

fprintf ('\ntestperf:  all tests passed.  Total time %g\n', toc (t)) ;

