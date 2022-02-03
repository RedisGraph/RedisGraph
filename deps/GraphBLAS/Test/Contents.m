% GraphBLAS Test/ folder: test GraphBLAS
% See the README.txt file for more details.

% Primary functiuns

%   make          - compiles the interface to GraphBLAS (for testing only)
%   testall       - run all GraphBLAS tests
%   nthreads_get  - get # of threads and chunk to use in GraphBLAS
%   nthreads_set  - set # of threads and chunk to use in GraphBLAS

% mimics of GraphBLAS operations:
%
%   GB_spec_Col_assign            - a mimic of GrB_Col_assign
%   GB_spec_Col_extract           - a mimic of GrB_Col_extract
%   GB_spec_Matrix_extract        - a mimic of GrB_Matrix_extract
%   GB_spec_Matrix_extractElement - a mimic of GrB_Matrix_extractElement
%   GB_spec_Row_assign            - a mimic of GrB_Row_assign
%   GB_spec_Vector_extract        - a mimic of GrB_Vector_extract
%   GB_spec_Vector_extractElement - a mimic of GrB_Matrix_extractElement
%   GB_spec_accum                 - mimic of the Z=accum(C,T) operation in GraphBLAS
%   GB_spec_accum_mask            - apply the accumulator and mask
%   GB_spec_apply                 - a mimic of GrB_apply
%   GB_spec_assign                - a mimic of GrB_assign (but not Row or Col variants)
%   GB_spec_build                 - a version of GrB_Matrix_build and GrB_vector_build
%   GB_spec_compare               - compare mimic result with GraphBLAS result
%   GB_spec_descriptor            - return components of a descriptor
%   GB_spec_Matrix_eWiseAdd       - a mimic of GrB_Matrix_eWiseAdd
%   GB_spec_Vector_eWiseAdd       - a mimic of GrB_Vector_eWiseAdd
%   GB_spec_Matrix_eWiseMult      - a mimic of GrB_Matrix_eWiseMult
%   GB_spec_Vector_eWiseMult      - a mimic of GrB_Vector_eWiseMult
%   GB_spec_extractTuples         - a mimic of GrB_*_extractTuples
%   GB_spec_identity              - the additive identity of a monoid
%   GB_spec_kron                  - a mimic of GrB_kronecker
%   GB_spec_mask                  - a pure implementation of GrB_mask
%   GB_spec_matrix                - a mimic that conforms a matrix to the GraphBLAS spec
%   GB_spec_mxm                   - a mimic of GrB_mxm
%   GB_spec_mxv                   - a mimic of GrB_mxv
%   GB_spec_op                    - apply a unary or binary operator
%   GB_spec_operator              - get the contents of an operator
%   GB_spec_opsall                - return a list of all operators, types, and semirings
%   GB_spec_random                - generate random matrix
%   GB_spec_reduce_to_scalar      - a mimic of GrB_reduce (to scalar)
%   GB_spec_reduce_to_vector      - a mimic of GrB_reduce (to vector)
%   GB_spec_resize                - a mimic of GxB_resize
%   GB_spec_select                - a mimic of GxB_select
%   GB_spec_semiring              - create a semiring
%   GB_spec_subassign             - a mimic of GxB_subassign
%   GB_spec_transpose             - a mimic of GrB_transpose
%   GB_spec_vxm                   - a mimic of GrB_vxm
%   GB_complex_compare            - compare GraphBLAS results for complex types
%   GB_user_op                    - apply a complex binary and unary operator
%   GB_user_opsall                - return list of complex operators
%   accum_mask                    - apply the mask
%   accum_mask2                   - a simpler version of GB_spec_accum_mask
%   GB_random_mask                - Mask = GB_random_mask (m, n, d, M_is_csc, M_is_hyper)
%   GB_spec_getmask               - return the mask, typecasted to logical

% Test scripts:

%   test01   - test GraphBLAS error handling
%   test02   - test GrB_*_dup
%   test03   - test GB_*_check functions
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
%   test27   - test GxB_select with user-defined select op (LoHi_band)
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
%   test54   - test GB_subref (numeric case) with I=lo:hi, J=lo:hi
%   test55   - test GxB_subassign, illustrate duplicate indices
%   test55b  - test GrB_assign, illustrate duplicate indices
%   test56   - test GrB_*_build
%   test57   - test operator on large uint32 values
%   test58   - test GrB_eWiseAdd
%   test59   - test GrB_mxm
%   test60   - test min and max operators with NaNs
%   test61   - performance test of GrB_eWiseMult
%   test62   - test GrB_apply
%   test63   - test GraphBLAS binary operators
%   test64   - test GxB_*_subassign, scalar expansion, with and without duplicates
%   test64b  - test GrB_*_assign, scalar expansion, with and without duplicates
%   test65   - test type casting
%   test66   - test GrB_reduce
%   test67   - test GrB_apply
%   test68   - performance tests for eWiseMult
%   test69   - test GrB_assign with aliased inputs, C<C>(:,:) = accum(C(:,:),C)
%   test72   - special cases for mxm, ewise, ...
%   test73   - performance of C = A*B, with mask
%   test74   - test GrB_mxm: all built-in semirings
%   test75   - test GrB_mxm and GrB_vxm on all semirings
%   test75b  - GrB_mxm and GrB_vxm on all semirings (shorter test than test75)
%   test76   - test GxB_resize
%   test77   - test GrB_kronecker
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
%   test88   - test hypersparse matrices with hash-based method
%   test89   - performance test of complex A*B
%   test90   - test AxB with user-defined semirings: plus_rdiv and plus_rdiv2
%   test91   - test subref performance on dense vectors
%   test92   - test GB_subref (symbolic case)
%   test95   - performance test for GrB_transpose
%   test96   - test dot product
%   test97   - test GB_assign, scalar expansion and zombies
%   test98   - test GrB_mxm, typecasting on the fly
%   test99   - test GB_mex_transpose with explicit zeros in the Mask
%   test101  - test import/export
%   test102  - test GB_AxB_saxpy3_flopcount
%   test103  - test aliases in GrB_transpose
%   test104  - export/import
%   test105  - eWiseAdd with hypersparse matrices
%   test106  - GxB_subassign with alias
%   test107  - user-defined terminal monoid
%   test108  - test boolean monoids
%   test109  - terminal monoid with user-defined type
%   test110  - test accum/mask (binary search of M(:,j))
%   test111  - performance test for eWiseAdd
%   test112  - test row/col scale
%   test113  - performance tests for GrB_kron
%   test114  - performance of reduce-to-scalar
%   test115  - test GB_assign, scalar expansion and zombies, with duplicates
%   test116  - performance tests for GrB_assign
%   test117  - performance tests for GrB_assign
%   test118  - performance tests for GrB_assign
%   test119  - performance tests for GrB_assign
%   test120  - performance tests for GrB_assign
%   test121  - performance tests for GrB_assign
%   test122  - performance tests for GrB_assign
%   test124  - GrB_extract, trigger case 6
%   test125  - test GrB_mxm: row and column scaling
%   test126  - test GrB_reduce to vector on a very sparse matrix 
%   test127  - test GrB_eWiseAdd and GrB_eWiseMult (all types and operators)
%   test128  - test eWiseMult and eWiseAdd, special cases
%   test129  - test GxB_select (tril and nonzero, hypersparse)
%   test130  - test GrB_apply (hypersparse cases)
%   test131  - test GrB_Matrix_clear
%   test132  - test GrB_*_setElement and GrB_*_*build
%   test133  - test mask operations (GB_masker)
%   test134  - test GxB_select
%   test135  - reduce-to-scalar, built-in monoids with terminal values
%   test136  - GxB_subassign, method 08, 09, 11
%   test137  - GrB_eWiseMult with FIRST and SECOND operators
%   test138  - test assign, with coarse-only tasks in IxJ slice
%   test139  - merge sort, special cases
%   test140  - test assign with duplicates
%   test141  - test GrB_eWiseAdd (all types and operators) for dense matrices
%   test142  - test GrB_assign for dense matrices
%   test143  - test special cases for C<!M>=A*B and C<M>=A*B
%   test144  - test GB_cumsum
%   test145  - test dot4
%   test146  - test C<M,struct> = scalar
%   test147  - test C<M>A*B with very sparse M
%   test148  - eWiseAdd with aliases
%   test149  - test fine hash method for C<!M>=A*B
%   test150  - test GrB_mxm with typecasting and zombies (dot3 and saxpy)
%   test151  - test bitwise operators
%   test152  - test C = A+B for dense A, B, and C
%   test153  - list all possible semirings
%   test154  - test GrB_apply with scalar binding
%   test155  - test GrB_*_setElement and GrB_*_removeElement
%   test156  - test assign C=A with typecasting

% TODO:: add new tests here

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

% Other tests:

%   t74       - run test20 and test74
%   testperf  - run all performance tests
%   atest     - test GrB_assign and GxB_subassign
%   atest11   - test GrB_assign and GxB_subassign
%   btest     - test GrB_build
%   etest     - test eWise
%   ee        - eWiseMult and eWiseAdd performance tests
%   grbinfo   - print info about the GraphBLAS version
%   mtest     - test mxm
%   longtests - very long tests

%   rtest     - test GrB_reduce to vector and scalar
%   ss        - test GxB_select
%   stest     - test GxB_select
%   testall2  - run testall with different # of threads
%   testall3  - run testall with different # of threads
%   tt        - test eWiseMult and A+B
%   ttest     - test GrB_extractTuples
%   xtest     - test GrB_extract
%   ztest     - test zombie deletion
%   testsort  - test qsort and msort

% Helper functions

%   debug_off        - turn off malloc debugging
%   debug_on         - turn on malloc debugging

%   irand            - construct a random integer matrix 
%   logstat          - run a GraphBLAS test and log the results to log.txt 
%   runtest          - run a single GraphBLAS test
%   stat             - report status of statement coverage and malloc debugging
%   GB_define        - create C source code for GraphBLAS.h

%   isequal_roundoff - compare two matrices, allowing for roundoff errors

%   test_other       - installs all packages needed for extensive tests

%   grb_clear_coverage - clear current statement coverage
%   grb_get_coverage   - return current statement coverage

%   flopcount        - cumulative sum of flop counts for A*B, C<M>=A*B, C<!M>=A*B
%   floptest         - compare flopcount with GB_mex_mxm_flops

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

