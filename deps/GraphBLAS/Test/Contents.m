% GraphBLAS Test/ folder: test GraphBLAS in MATLAB
% See the README.txt file for more details.
%
% MATLAB mimics of GraphBLAS operations:
%
%   GB_spec_Col_assign            - a MATLAB mimic of GrB_Col_assign
%   GB_spec_Col_extract           - a MATLAB mimic of GrB_Col_extract
%   GB_spec_Matrix_extract        - a MATLAB mimic of GrB_Matrix_extract
%   GB_spec_Matrix_extractElement - a MATLAB mimic of GrB_Matrix_extractElement
%   GB_spec_Row_assign            - a MATLAB mimic of GrB_Row_assign
%   GB_spec_Vector_extract        - a MATLAB mimic of GrB_Vector_extract
%   GB_spec_Vector_extractElement - a MATLAB mimic of GrB_Matrix_extractElement
%   GB_spec_accum                 - MATLAB mimic of the Z=accum(C,T) operation in GraphBLAS
%   GB_spec_accum_mask            - apply the accumulator and mask
%   GB_spec_apply                 - a MATLAB mimic of GrB_apply
%   GB_spec_assign                - a MATLAB mimic of GrB_assign (but not Row or Col variants)
%   GB_spec_build                 - a MATLAB version of GrB_Matrix_build and GrB_vector_build
%   GB_spec_compare               - compare MATLAB mimic result with GraphBLAS result
%   GB_spec_descriptor            - return components of a descriptor
%   GB_spec_eWiseAdd_Matrix       - a MATLAB mimic of GrB_eWiseAdd_Matrix
%   GB_spec_eWiseAdd_Vector       - a MATLAB mimic of GrB_eWiseAdd_Vector
%   GB_spec_eWiseMult_Matrix      - a MATLAB mimic of GrB_eWiseMult_Matrix
%   GB_spec_eWiseMult_Vector      - a MATLAB mimic of GrB_eWiseMult_Vector
%   GB_spec_extractTuples         - a MATLAB mimic of GrB_*_extractTuples
%   GB_spec_identity              - the additive identity of a monoid
%   GB_spec_kron                  - a MATLAB mimic of GxB_kron
%   GB_spec_mask                  - a pure MATLAB implementation of GrB_mask
%   GB_spec_matrix                - a MATLAB mimic that conforms a matrix to the GraphBLAS spec
%   GB_spec_mxm                   - a MATLAB mimic of GrB_mxm
%   GB_spec_mxv                   - a MATLAB mimic of GrB_mxv
%   GB_spec_op                    - apply a unary or binary operator
%   GB_spec_operator              - get the contents of an operator
%   GB_spec_opsall                - return a list of all operators, classes, and semirings
%   GB_spec_random                - generate random matrix
%   GB_spec_reduce_to_scalar      - a MATLAB mimic of GrB_reduce (to scalar)
%   GB_spec_reduce_to_vector      - a MATLAB mimic of GrB_reduce (to vector)
%   GB_spec_resize                - a MATLAB mimic of GxB_resize
%   GB_spec_select                - a MATLAB mimic of GxB_select
%   GB_spec_semiring              - create a semiring
%   GB_spec_subassign             - a MATLAB mimic of GxB_subassign
%   GB_spec_transpose             - a MATLAB mimic of GrB_transpose
%   GB_spec_vxm                   - a MATLAB mimic of GrB_vxm
%   GB_user_compare               - compare GraphBLAS results for complex types
%   GB_user_op                    - apply a complex binary and unary operator
%   GB_user_opsall                - return list of complex operators
%   accum_mask                    - apply the mask
%   accum_mask2                   - a simpler version of GB_spec_accum_mask
%   GB_random_mask                - Mask = GB_random_mask (m, n, d, M_is_csc, M_is_hyper)
%   GB_spec_getmask               - return the mask, typecasted to logical
%
% Test scripts:
%
%   testall  - run all GraphBLAS tests
%   test00   - test GB_mex_mis
%   test01   - test GraphBLAS error handling
%   test02   - test GrB_*_dup
%   test03   - test GB_check functions
%   test04   - test and demo for accumulator/mask and transpose
%   test05   - test GrB_*_setElement
%   test06   - test GrB_mxm on all semirings
%   test07   - test GxB_subassign with a single pending tuple
%   test07b  - test GrB_assign with a single pending tuple
%   test08   - test GxB_subassign
%   test08b  - test GrB_assign
%   test09   - test GxB_subassign
%   test09b  - test GrB_assign
%   test10   - test GrB_apply
%   test11   - test GrB_*_extractTuples
%   test12   - test Wathen matrix generation
%   test13   - test GrB_tranpsose
%   test14   - test GrB_reduce
%   test15   - test AxB and AdotB internal functions
%   test16   - test user-defined complex type (runs all testc*.m)
%   test17   - test GrB_*_extractElement
%   test18   - test GrB_eWiseAdd and GrB_eWiseMult
%   test19   - test GxB_subassign and GrB_*_setElement with many pending operations
%   test19b  - test GrB_assign and GrB_*_setElement with many pending operations
%   test20   - test GrB_mxm, mxv, and vxm
%   test21   - test GxB_subassign
%   test21b  - test GrB_assign
%   test22   - test GrB_transpose
%   test23   - test GrB_*_build
%   test24   - test GrB_reduce
%   test25   - test GxB_select
%   test26   - performance test for GxB_select
%   test27   - test GxB_select with user-defined select op (band)
%   test28   - test mxm with aliased inputs, C<C> = accum(C,C*C)
%   test29   - GrB_reduce with zombies
%   test30   - test GxB_subassign
%   test30b  - performance test GB_mex_assign, scalar expansionb
%   test31   - test GrB_transpose
%   test32   - test GrB_mxm
%   test33   - test a semiring
%   test34   - test GrB_eWiseAdd
%   test35   - test GrB_*_extractTuples
%   test36   - performance test of matrix subref
%   test37   - test qsort
%   test38   - test GrB_transpose
%   test39   - performance test for GrB_transpose
%   test40   - test GrB_Matrix_extractElement
%   test41   - test AxB
%   test42   - test GrB_Matrix_build
%   test43   - test subref
%   test44   - test qsort
%   test45   - test GrB_*_setElement and GrB_*_*build
%   test46   - performance test of GxB_subassign
%   test46b  - performance test of GrB_assign
%   test47   - prformance test of GrB_vxm
%   test48   - performance test of GrB_mxm
%   test49   - performance test of GrB_mxm (dot product method, A'*B)
%   test50   - test AxB numeric and symbolic
%   test51   - test GxB_subassign, multiply operations
%   test51b  - test GrB_assign, multiply operations
%   test52   - test AdotB vs AxB
%   test53   - test GrB_Matrix_extract
%   test54   - test GB_subref_numeric with I=lo:hi, J=lo:hi
%   test55   - test GxB_subassign, illustrate duplicate indices, MATLAB vs GraphBLAS
%   test55b  - test GrB_assign, illustrate duplicate indices, MATLAB vs GraphBLAS
%   test56   - test GrB_*_build
%   test57   - test operator on large uint32 values
%   test58   - test GrB_eWiseAdd
%   test59   - test GrB_mxm
%   test60   - test min and max operators with NaNs
%   test61   - performance test of GrB_eMult
%   test62   - test GrB_apply
%   test63   - test GraphBLAS operators
%   test64   - test GxB_*_subassign, scalar expansion, with and without duplicates
%   test64b  - test GrB_*_assign, scalar expansion, with and without duplicates
%   test65   - test type casting
%   test66   - test GrB_reduce
%   test67   - test GrB_apply
%   test68   - performance tests for eWiseMult
%   test69   - test GrB_assign with aliased inputs, C<C>(:,:) = accum(C(:,:),C)
%   test72   - special cases for mxm, ewise, ...
%   test73   - performance of C = A*B, with mask
%   test74   - test GrB_mxm: dot product method
%   test75   - test GrB_mxm and GrB_vxm on all semirings (A'B dot product)
%   test76   - test GxB_resize
%   test77   - test GxB_kron
%   test78   - test subref
%   test79   - run all matrices with test06
%   test80   - rerun test06 with different matrices
%   test81   - test GrB_Matrix_extract with index range, stride, & backwards
%   test82   - test GrB_Matrix_extract with index range (hypersparse)
%   test83   - test GrB_assign with J=lo:0:hi, an empty list, and C_replace true
%   test84   - test GrB_assign (row and column with C in CSR format)
%   test85   - test GrB_transpose: 1-by-n with typecasting
%   test86   - performance test of of GrB_Matrix_extract
%   test87   - performance test of GrB_mxm
%   test88   - test hypersparse matrices with heap-based method
%   test89   - performance test of complex A*B
%   test90   - test AxB with pre-compiled semirings: plus_rdiv and plus_rdiv2
%   test91   - test subref performance on dense vectors
%   test92   - test GB_subref_symbolic
%   test93   - test dpagerank and ipagerank
%   test94   - test pagerank
%   test97   - test GB_assign, scalar expansion and zombies
%   test98   - test GrB_mxm, typecasting on the fly
%   test99   - test GB_mex_transpose with explicit zeros in the Mask
%   testc1   - test complex operators
%   testc2   - test complex A*B, A'*B, A*B', A'*B', A+B
%   testc3   - test complex GrB_extract
%   testc4   - test complex extractElement and setElement
%   testc5   - test complex subref
%   testc6   - test complex apply
%   testc7   - test complex assign
%   testc8   - test complex eWiseAdd and eWiseMult
%   testc9   - test complex extractTuples
%   testca   - test complex mxm, mxv, and vxm
%   testcb   - test complex reduce
%   testcc   - test complex transpose
%   testperf - run all performance tests
%
% Helper functions
%
%   debug_off  - turn off malloc debugging
%   debug_on   - turn on malloc debugging
%   gbmake     - compiles the MATLAB interface to GraphBLAS (for testing only).
%   irand      - construct a random integer matrix 
%   logstat    - run a GraphBLAS test and log the results to log.txt 
%   runtest    - run a single GraphBLAS test
%   stat       - report status of statement coverage and malloc debugging
%   GB_define  - create C source code for GraphBLAS.h
%   GB_define2 - construct part of the GB.h file, to allow user-defined objects
%   gbresults  - return time taken by last GraphBLAS function, and AxB method
%   isequal_roundoff - compare two matrices, allowing for roundoff errors
%   startup    - setup the path for tests in GraphBLAS/Test
%   test_other - installs all packages needed for extensive tests
%
% Triangle counting:
%
%   ../Demo/MATLAB/tricount       - count the number of triangles in an undirected unweighted graph
%   ../Demo/MATLAB/adj_to_edges   - create an edge incidence matrix from an adjacency matrix
%   ../Demo/MATLAB/check_adj      - ensure A is a valid adjacency matrix
%   ../Demo/MATLAB/edges_to_adj   - create an adjacency matrix from an edge incidence matrix
%   ../Demo/MATLAB/tri_matlab     - run tricount tests in MATLAB
%   test70       - performance comparison of triangle counting methods
%   test70_plot  - plot the results from test70
%   test71       - performance comparison of triangle counting methods
%   test71_plot  - plot the results from test71
%   test71_table - print the table for triangle counting results
%
% Other demos
%
%   ../Demo/MATLAB/kron_demo      - test Program/kron_demo.c and compare with MATLAB kron
%   ../Demo/MATLAB/kron_test      - test kron_demo.m
