%LONGTESTS very long tests

nthreads_set (4,1) ;
debug_off 

    test21(1) ;  % exhaustive test of GB_mex_subassign
    test23(1) ;  % exhaustive test of GB_*_build
    test24(1) ;  % exhaustive test of GrB_Matrix_reduce

    test30b ; % performance test GB_mex_assign, scalar expansion

    test12(0) ; % Wathen finite-element matrices (full test)
    test58(0) ; % longer GB_mex_eWiseAdd_Matrix performance test

    test53 ;  % exhaustive test of GB_mex_Matrix_extract
    test62 ;  % exhaustive test of GrB_apply
    test45 ;  % test GB_mex_setElement and build
    test46 ;  % performance test GB_mex_subassign
    test46b ; % performance test GB_mex_assign

    test51 ;  % performance test GB_mex_subassign, multiple ops
    test51b ; % performance test GB_mex_assign, multiple ops
    test06(936) ; % performance test of GrB_mxm on all semirings

