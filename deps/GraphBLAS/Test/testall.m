function testall (threads,longtests)
%TESTALL run all GraphBLAS tests
%
% Usage:
% testall ;             % runs just the shorter tests (about 30 minutes)
%
% testall(threads) ;    % run with specific list of threads and chunk sizes
% testall([ ],1) ;      % run all longer tests, with default # of threads
%
% threads is a cell array. Each entry is 2-by-1, with the first value being
% the # of threads to use and the 2nd being the chunk size.  The default is
% {[4 1]} if empty or not present.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

GrB.init

testall_time = tic ;

if (nargin < 2)
    % run the shorter tests by default
    longtests = 0 ;
end

if (nargin < 1)
    threads = [ ] ;
end
if (isempty (threads))
    threads {1} = [4 1] ;
end
t = threads ;

% single thread
s {1} = [1 1] ;

extra {1} = [4 1] ;
extra {2} = [1 1] ;

% clear the statement coverage counts
clear global GraphBLAS_grbcov

% use built-in complex data types by default
GB_builtin_complex_set (true) ;

% many of the tests use spok in SuiteSparse, a copy of which is
% included here in GraphBLAS/Test/spok.
addpath ('../Test/spok') ;

try
    spok (sparse (1)) ;
catch
    here = pwd ;
    cd ../Test/spok ;
    spok_install ;
    cd (here) ;
end

logstat ;             % start the log.txt
hack = GB_mex_hack ;

% start with the Werk stack enabled
hack (2) = 0 ;
GB_mex_hack (hack) ;

%===============================================================================
% quick tests for statement coverage, with malloc debugging
%===============================================================================

% Timings below are for test coverage (Tcov), with malloc debuging enabled, on
% hypersparse.cse.tamu.edu (20 core Xeon).  Times will differ if this test is
% run with malloc debugging off.

%----------------------------------------
% tests with high rates (over 100/sec)
%----------------------------------------

logstat ('test246',t) ; % GrB_mxm parallelism (changes slice_balanced)
logstat ('test01' ,t) ; % error handling
logstat ('test245',t) ; % test complex row/col scale
logstat ('test199',t) ; % test dot2 with hypersparse
logstat ('test83' ,t) ; % GrB_assign with C_replace and empty J
logstat ('test210',t) ; % test iso assign25: C<M,struct>=A, C empty, A dense
logstat ('test165',t) ; % test C=A*B' where A is diagonal and B becomes bitmap
logstat ('test219',s) ; % test reduce to scalar (1 thread)
logstat ('test241',t) ; % test GrB_mxm, triggering the swap_rule
logstat ('test220',t) ; % test mask C<M>=Z, iso case
logstat ('test211',t) ; % test iso assign
logstat ('test202',t) ; % test iso add and emult
logstat ('test152',t) ; % test binops with C=A+B, all matrices dense
logstat ('test222',t) ; % test user selectop for iso matrices

hack (2) = 1 ; GB_mex_hack (hack) ; % disable the Werk stack
logstat ('test240',t) ; % test dot4 and saxpy5
logstat ('test186',t) ;     % saxpy, all sparsity formats  (slice_balanced)
logstat ('test186(0)',t) ;  % repeat with default slice_balanced
logstat ('test186',s) ;     % repeat, but single-threaded
logstat ('test150',t) ; % mxm with zombies and typecasting (dot3 and saxpy)
hack (2) = 0 ; GB_mex_hack (hack) ; % re-enable the Werk stack

logstat ('test239',t) ; % test GxB_eWiseUnion
logstat ('test235',t) ; % test GxB_eWiseUnion and GrB_eWiseAdd
logstat ('test226',t) ; % test kron with iso matrices
logstat ('test223',t) ; % test matrix multiply, C<!M>=A*B
logstat ('test204',t) ; % test iso diag
logstat ('test203',t) ; % test iso subref
logstat ('test183',s) ; % test eWiseMult with hypersparse mask
logstat ('test179',t) ; % test bitmap select
logstat ('test174',t) ; % test GrB_assign C<A>=A
logstat ('test155',t) ; % test GrB_*_setElement and GrB_*_removeElement
logstat ('test156',t) ; % test GrB_assign C=A with typecasting
logstat ('test136',s) ; % subassignment special cases
logstat ('test02' ,t) ; % matrix copy and dup tests
logstat ('test109',t) ; % terminal monoid with user-defined type
logstat ('test109',s) ; % terminal monoid with user-defined type
logstat ('test04' ,t) ; % simple mask and transpose test
logstat ('test207',t) ; % test iso subref
logstat ('test221',t) ; % test C += A where C is bitmap and A is full
logstat ('test162',t) ; % test C<M>=A*B with very sparse M
logstat ('test159',t) ; % test A*B
logstat ('test09' ,t) ; % duplicate I,J test of GB_mex_subassign
logstat ('test132',t) ; % setElement
logstat ('test141',t) ; % eWiseAdd with dense matrices
logstat ('testc2(1,1)',t) ; % complex tests (quick case, builtin)
logstat ('test214',t) ; % test C<M>=A'*B (tricount)
logstat ('test213',t) ; % test iso assign (method 05d)
logstat ('test206',t) ; % test iso select
logstat ('test212',t) ; % test iso mask all zero
logstat ('test128',t) ; % eWiseMult, eWiseAdd, eWiseUnion special cases
logstat ('test82' ,t) ; % GrB_extract with index range (hypersparse)

%----------------------------------------
% tests with good rates (30 to 100/sec)
%----------------------------------------

logstat ('test229',t) ; % test setElement
logstat ('test144',t) ; % cumsum

%----------------------------------------
% tests with decent rates (20 to 30/sec)
%----------------------------------------

hack (2) = 1 ; GB_mex_hack (hack) ; % disable the Werk stack
logstat ('test14' ,t) ; % GrB_reduce
logstat ('test180',s) ; % test assign and subassign (single threaded)
logstat ('test236',t) ; % test GxB_Matrix_sort and GxB_Vector_sort
hack (2) = 0 ; GB_mex_hack (hack) ; % re-enable the Werk stack

%----------------------------------------
% tests with decent rates (10 to 20/sec)
%----------------------------------------

logstat ('test232',t) ; % test assign with GrB_Scalar
logstat ('test228',t) ; % test serialize/deserialize

%----------------------------------------
% tests with low coverage/sec rates (1/sec to 10/sec)
%----------------------------------------

hack (2) = 1 ; GB_mex_hack (hack) ; % disable the Werk stack
logstat ('test154',t) ; % apply with binop and scalar binding
logstat ('test238',t) ; % test GrB_mxm (dot4 and dot2)
logstat ('test151b',t); % test bshift operator
logstat ('test184',t) ; % test special cases for mxm, transpose, and build
logstat ('test191',t) ; % test split
logstat ('test188',t) ; % test concat
logstat ('test237',t) ; % test GrB_mxm (saxpy4)
hack (2) = 0 ; GB_mex_hack (hack) ; % re-enable the Werk stack

logstat ('test224',t) ; % test unpack/pack
logstat ('test196',t) ; % test hypersparse concat
logstat ('test209',t) ; % test iso build
logstat ('test104',t) ; % export/import

%----------------------------------------
% tests with very low coverage/sec rates  (< 1/sec)
%----------------------------------------

logstat ('test189',t) ; % test large assign
logstat ('test194',t) ; % test GxB_Vector_diag
logstat ('test76' ,s) ; % GxB_resize (single threaded)
logstat ('test244',t) ; % test GxB_Matrix_reshape*

%===============================================================================
% tests with no malloc debugging
%===============================================================================

% Turn off malloc debugging
malloc_debugging = stat ;
if (malloc_debugging)
    debug_off
    fprintf ('[malloc debugging turned off]\n') ;
    f = fopen ('log.txt', 'a') ;
    fprintf (f, '[malloc debugging turned off]\n') ;
    fclose (f) ;
end

%----------------------------------------
% tests with good rates (30 to 100/sec)
%----------------------------------------

logstat ('test201',t) ; % test iso reduce to vector
logstat ('test225',t) ; % test mask operations (GB_masker)
logstat ('test170',t) ; % test C<B>=A+B (alias M==B)
logstat ('test176',t) ; % test GrB_assign, method 09, 11
logstat ('test208',t) ; % test iso apply, bind 1st and 2nd
logstat ('test216',t) ; % test C<A>=A, iso case
logstat ('test142',t) ; % test GrB_assign with accum
logstat ('test137',s) ; % GrB_eWiseMult with FIRST and SECOND operators
logstat ('test139',s) ; % merge sort, special cases
logstat ('test145',t) ; % dot4 for C += A'*B
logstat ('test172',t) ; % test eWiseMult with M bitmap/full
logstat ('test148',t) ; % ewise with alias

%----------------------------------------
% tests with decent rates (20 to 30/sec)
%----------------------------------------

logstat ('test157',t) ; % test sparsity formats
logstat ('test182',s) ; % test for internal wait

%----------------------------------------
% tests with decent rates (10 to 20/sec)
%----------------------------------------

logstat ('test108',t) ; % boolean monoids
logstat ('test130',t) ; % GrB_apply, hypersparse cases
logstat ('test124',t) ; % GrB_extract, case 6
logstat ('test138',s) ; % test assign, with coarse-only tasks in IxJ slice
logstat ('test227',t) ; % test kron
logstat ('test125',t) ; % test GrB_mxm: row and column scaling

%----------------------------------------
% 1 to 10/sec
%----------------------------------------

logstat ('test234',t) ; % test GxB_eWiseUnion
logstat ('test242',t) ; % test GxB_Iterator for matrices
logstat ('test173',t) ; % test GrB_assign C<A>=A
logstat ('test200',t) ; % test iso full matrix multiply
logstat ('test197',t) ; % test large sparse split
logstat ('test84' ,t) ; % GrB_assign (row and column with C in CSR/CSC format)
logstat ('test19b',t) ; % GrB_assign, many pending operators
logstat ('test19b',s) ; % GrB_assign, many pending operators
logstat ('test133',t) ; % test mask operations (GB_masker)
logstat ('test80' ,t) ; % test GrB_mxm on all semirings (different matrix)
logstat ('test151',t) ; % test bitwise operators
logstat ('test23' ,t) ; % quick test of GB_*_build
logstat ('test135',t) ; % reduce to scalar
logstat ('test160',s) ; % test A*B, single threaded
logstat ('test54' ,t) ; % assign and extract with begin:inc:end
logstat ('test129',t) ; % test GxB_select (tril and nonzero, hypersparse)
logstat ('test69' ,t) ; % assign and subassign with alias
logstat ('test230',t) ; % test apply with idxunops
logstat ('test74' ,t) ; % test GrB_mxm on all semirings
logstat ('test127',t) ; % test eWiseAdd, eWiseMult (all types and operators)
logstat ('test19',t) ;  % GxB_subassign, many pending operators

%----------------------------------------
% < 1 per sec
%----------------------------------------

logstat ('test11' ,t) ; % exhaustive test of GrB_extractTuples
logstat ('test160',t) ; % test A*B, parallel
logstat ('test215',t) ; % test C<M>=A'*B (dot2, ANY_PAIR semiring)
logstat ('test193',t) ; % test GxB_Matrix_diag
logstat ('test195',t) ; % test all variants of saxpy3 (changes slice_balanced)
logstat ('test233',t) ; % test bitmap saxpy C=A*B with A sparse and B bitmap
logstat ('test243',t) ; % test GxB_Vector_Iterator
logstat ('test29' ,t) ; % reduce with zombies

logstat ('testc2(0,0)',t) ;  % A'*B, A+B, A*B, user-defined complex type
logstat ('testc4(0)',t) ;  % extractElement, setElement, user-defined complex
logstat ('testc7(0)',t) ;  % assign, builtin complex
logstat ('testcc(1)',t) ;  % transpose, builtin complex

hack (2) = 1 ; GB_mex_hack (hack) ; % disable the Werk stack
logstat ('test187',t) ; % test dup/assign for all sparsity formats
logstat ('test192',t) ; % test C<C,struct>=scalar
logstat ('test181',s) ; % test transpose with explicit zeros in the mask
logstat ('test185',s) ; % test dot4, saxpy for all sparsity formats
hack (2) = 0 ; GB_mex_hack (hack) ; % re-enable the Werk stack

logstat ('test53' ,t) ; % quick test of GB_mex_Matrix_extract
logstat ('test17' ,t) ; % quick test of GrB_*_extractElement
logstat ('test231',t) ; % test GrB_select with idxunp

%----------------------------------------
% longer tests (200 seconds to 600 seconds, or low rate of coverage)
%----------------------------------------

logstat ('test10' ,t) ; % GrB_apply
logstat ('test75b',t) ; % test GrB_mxm A'*B (quicker than test75)
logstat ('test21b',t) ; % quick test of GB_mex_assign
logstat ('testca(1)',t) ;  % test complex mxm, mxv, and vxm
logstat ('test81' ,t) ; % GrB_Matrix_extract with stride, range, backwards
logstat ('test18' ,t) ; % quick tests of GrB_eWiseAdd and eWiseMult

%===============================================================================
% The following tests are not required for statement coverage.  Some need
% other packages in SuiteSparse (CSparse, SSMULT, ssget).  By default, these
% tests are not run.  To install them, see test_other.m.  Timing is with malloc
% debugging turned off.

if (longtests)

% ------------------------ % ---- % ------------------------------
% test script              % time % description
% ------------------------ % ---- % ------------------------------

logstat ('test03' ,t) ;    %    0 % random matrix tests
logstat ('test03' ,s) ;    %    0 % random matrix tests
logstat ('test05',t) ;     %      % quick setElement test, with typecasting
logstat ('test06(936)',t); %      % performance test GrB_mxm on all semirings
logstat ('test07',t) ;     %    0 % quick test GB_mex_subassign
logstat ('test07',s) ;     %    0 % quick test GB_mex_subassign
logstat ('test07b',t) ;    %      % quick test GB_mex_assign
logstat ('test09b',t) ;    %      % duplicate I,J test of GB_mex_assign

logstat ('test13',t) ;     %      % simple tests of GB_mex_transpose
logstat ('test15',t) ;            % simple test of GB_mex_AxB
logstat ('test16' ,t) ;    %  177 % user-defined complex operators

logstat ('test20',t) ;            % quick test of GB_mex_mxm on a few semirings
logstat ('test20(1)',t) ;  %      % test of GB_mex_mxm on all built-in semirings
logstat ('test21',s) ;     %   41 % quick test of GB_mex_subassign
logstat ('test21(1)',t) ;  %      % exhaustive test of GB_mex_subassign
logstat ('test22',t) ;     %      % quick test of GB_mex_transpose
logstat ('test23(1)',t) ;  %      % exhaustive test of GB_*_build
logstat ('test24',t) ;     %   42 % test of GrB_Matrix_reduce
logstat ('test24(1)',t) ;  %      % exhaustive test of GrB_Matrix_reduce
logstat ('test25',t) ;     %      % long test of GxB_select
logstat ('test26',t) ;     %   .6 % quick test of GxB_select
logstat ('test26(1)',t) ;  %      % performance test of GxB_select (use ssget)
logstat ('test27',t) ;     %   13 % quick test of GxB_select (LoHi_band)
logstat ('test28',t) ;     %    1 % mxm with aliased inputs, C<C> = accum(C,C*C)

logstat ('test30') ;       %   11 % GB_mex_subassign, scalar expansion
logstat ('test30b') ;      %    9 % performance GB_mex_assign, scalar expansion
logstat ('test31',t) ;     %      % simple tests of GB_mex_transpose
logstat ('test32',t) ;     %      % quick GB_mex_mxm test
logstat ('test33',t) ;     %      % create a semiring
logstat ('test34',t) ;     %      % quick GB_mex_Matrix_eWiseAdd test
logstat ('test35') ;       %      % performance test for GrB_extractTuples
logstat ('test36') ;       %      % performance test for GB_mex_Matrix_subref
logstat ('test38',t) ;     %      % GB_mex_transpose with matrix collection
logstat ('test39') ;       %      % GrB_transpose, GB_*_add and eWiseAdd
logstat ('test39(0)') ;    %   55 % GrB_transpose, GB_*_add and eWiseAdd

logstat ('test40',t) ;     %      % GrB_Matrix_extractElement, and Vector
logstat ('test41',t) ;     %      % test of GB_mex_AxB
logstat ('test42') ;       %      % performance tests for GB_mex_Matrix_build
logstat ('test43',t) ;     %      % performance tests for GB_mex_Matrix_subref
logstat ('test44',t) ;     %    5 % test qsort
logstat ('test45(0)',t) ;  %  334 % test GB_mex_setElement and build
logstat ('test46') ;       %      % performance test GB_mex_subassign
logstat ('test46b') ;      %      % performance test GB_mex_assign
logstat ('test47',t) ;     %      % performance test of GrB_vxm
logstat ('test48') ;       %      % performance test of GrB_mxm
logstat ('test49') ;       %      % performance test of GrB_mxm (dot, A'*B)

logstat ('test50',t) ;     %      % test GB_mex_AxB on larger matrix
logstat ('test51') ;       %      % performance test GB_mex_subassign
logstat ('test51b') ;      %      % performance test GB_mex_assign, multiple ops
logstat ('test52',t) ;     %      % performance of A*B with tall mtx, AdotB, AxB
logstat ('test53',t) ;     %      % exhaustive test of GB_mex_Matrix_extract
logstat ('test55',t) ;     %      % GxB_subassign, dupl, built-in vs GraphBLAS
logstat ('test55b',t) ;    %      % GrB_assign, duplicates, built-in vs GrB
logstat ('test56',t) ;     %      % test GrB_*_build
logstat ('test57',t) ;     %      % test operator on large uint32 values
logstat ('test58(0)') ;    %      % longer GB_mex_Matrix_eWiseAdd performance
logstat ('test58') ;       %      % test GrB_eWiseAdd
logstat ('test59',t) ;     %      % test GrB_mxm

logstat ('test60',t) ;     %      % test min and max operators with NaNs
logstat ('test61') ;       %      % performance test of GrB_eWiseMult
logstat ('test62',t) ;     %      % exhaustive test of GrB_apply
logstat ('test63',t) ;     %      % GB_mex_op and operator tests
logstat ('test64',t) ;     %      % GB_mex_subassign, scalar expansion
logstat ('test64b',t) ;    %      % GrB_*_assign, scalar expansion
logstat ('test65',t) ;     %      % test type casting
logstat ('test66',t) ;     %      % quick test for GrB_Matrix_reduce
logstat ('test67',t) ;     %      % quick test for GrB_apply
logstat ('test68',t) ;

logstat ('test72' ,t) ;    %    0 % several special cases
logstat ('test73',t) ;     %      % performance of C = A*B, with mask
logstat ('test75',t) ;     %      % test GrB_mxm A'*B on all semirings
logstat ('test77',t) ;     %  450 % long tests of GrB_kronecker
logstat ('test78',t) ;     %    1 % quick test of hypersparse subref
logstat ('test79',t) ;     %      % run all in SuiteSparse Collection

logstat ('test85',t) ;     %    0 % GrB_transpose (1-by-n with typecasting)
logstat ('test86',t) ;     %      % performance test of of GrB_Matrix_extract
logstat ('test87',t) ;     %      % performance test of GrB_mxm
logstat ('test88',t) ;            % hypersparse matrices with hash-based method
logstat ('test89',t) ;     %      % performance test of complex A*B

logstat ('test90',t) ;     %    1 % test user-defined semirings
logstat ('test91',t) ;     %      % test subref performance on dense vectors
logstat ('test92' ,t) ;    %   .1 % GB_subref: symbolic case
logstat ('test95',t) ;     %      % performance test for GrB_transpose
logstat ('test96',t) ;     %   16 % A*B using dot product
logstat ('test97',t) ;     %    0 % GB_mex_assign, scalar expansion and zombies
logstat ('test98',t) ;     %      % GB_mex_mxm, typecast on the fly
logstat ('test99',t) ;     %   20 % GB_mex_transpose w/ explicit 0s in the Mask

logstat ('test101',t) ;    %    1 % import and export
logstat ('test102',t);     %    1 % GB_AxB_saxpy3_flopcount
logstat ('test103',t) ;    %      % GrB_transpose aliases
logstat ('test105',t) ;    %    2 % eWiseAdd for hypersparse
logstat ('test106',t) ;    %    4 % GxB_subassign with alias
logstat ('test107',t) ;    %    2 % monoids with terminal values

logstat ('test110',t) ;    %    0 % binary search of M(:,j) in accum/mask
logstat ('test111',t) ;    %      % performance test for eWiseAdd
logstat ('test112',t) ;    %      % test row/col scale
logstat ('test113',t) ;    %      % performance tests for GrB_kron
logstat ('test114',t) ;    %      % performance of reduce-to-scalar
logstat ('test115',t) ;    %   10 % GrB_assign with duplicate indices
logstat ('test116',t) ;    %      % performance tests for GrB_assign
logstat ('test117',t) ;    %      % performance tests for GrB_assign
logstat ('test118',t) ;    %      % performance tests for GrB_assign
logstat ('test119',t) ;    %      % performance tests for GrB_assign

logstat ('test120',t) ;    %      % performance tests for GrB_assign
logstat ('test121',t) ;    %      % performance tests for GrB_assign
logstat ('test122',t) ;    %      % performance tests for GrB_assign
logstat ('test126',t) ;    %    7 % GrB_reduce to vector; very sparse matrix 

logstat ('test131',t) ;    %   .1 % GrB_Matrix_clear
logstat ('test134',t) ;    %  105 % quick test of GxB_select

logstat ('test143',t) ;    %   37 % mxm, special cases
logstat ('test146',t) ;    %   .1 % expand scalar
logstat ('test147',t) ;           % C<M>=A*B with very sparse M
logstat ('test149',t) ;           % test fine hash tasks for C<!M>=A*B

logstat ('test158',t) ;    %  10  % test colscale and rowscale

logstat ('test161',t) ;    %      % test A*B*E
logstat ('test163',t) ;    %   .6 % test C<!M>=A'*B where C and M are sparse
logstat ('test164',t) ;    %    0 % test dot5 method
logstat ('test166',t) ;    %   .1 % test GxB_select with a dense matrix
logstat ('test167',t) ;    %   .2 % test C<M>=A*B with very sparse M, different types
logstat ('test168',t) ;           % test C=A+B with C and B full, A bitmap
logstat ('test169',t) ;    %    0 % test C<!M>=A+B with C sparse, M hyper, A and B sparse

logstat ('test171',t) ;    %    1 % test conversion and GB_memset
logstat ('test175',t) ;    %    8 % test142 updated
logstat ('test177',t) ;    %  1.2 % test C<!M>=A*B, C and B bitmap, M and A sparse

logstat ('test180',t) ;    %  16  % test assign and subassign (multi threaded)

logstat ('test190',t) ;    %   .3 % test dense matrix for C<!M>=A*B
logstat ('test198',t) ;    %   .1 % test apply with C=op(C)

logstat ('test205',t) ;    %    0 % test iso kron
logstat ('test217',t) ;    %    0 % test C<repl>(I,J)=A, bitmap assign
logstat ('test218',t) ;    %    0 % test C=A+B, C and A are full, B is bitmap

logstat ('testc1',t) ;     %      % test complex operators
logstat ('testc2',t) ;     %      % test complex A*B, A'*B, A*B', A'*B', A+B
logstat ('testc3',t) ;     %      % test complex GrB_extract
logstat ('testc4',t) ;     %      % test complex extractElement and setElement
logstat ('testc5',t) ;     %      % test complex subref
logstat ('testc6',t) ;     %      % test complex apply
logstat ('testc7',t) ;     %      % test complex assign
logstat ('testc8',t) ;     %      % test complex eWiseAdd and eWiseMult
logstat ('testc9',t) ;     %      % test complex extractTuples
logstat ('testca',t) ;     %      % test complex mxm, mxv, and vxm
logstat ('testcb',t) ;     %      % test complex reduce
logstat ('testcc',t) ;     %      % test complex transpose

end

if (malloc_debugging)
    debug_on
    fprintf ('[malloc debugging turned back on]\n') ;
    f = fopen ('log.txt', 'a') ;
    fprintf (f, '[malloc debugging turned back on]\n') ;
    fclose (f) ;
end

t = toc (testall_time) ;
fprintf ('\ntestall: all tests passed, total time %0.4g minutes\n', t / 60) ;

