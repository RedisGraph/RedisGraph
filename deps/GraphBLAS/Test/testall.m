function testall (threads,longtests)
%TESTALL run all GraphBLAS tests
%
% Usage:
% testall ;         % runs just the shorter tests (about 15 minutes)
%
% testall(threads) ;    % run with specific list of threads and chunk sizes
%
% threads is a cell array. Each entry is 2-by-1, with the first value being
% the # of threads to use and the 2nd being the chunk size.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

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
%-------------------------------------------------------------------------------

% Timings below are for test coverage (Tcov), with malloc debuging enabled, on
% hypersparse.cse.tamu.edu (20 core Xeon).  Times will differ if this test is
% run in with malloc debugging off.

%----------------------------------------
% test taking less than 1 second:
%----------------------------------------

%0
logstat ('test136',s) ; % subassignment special cases
%0
logstat ('test137',s) ; % GrB_eWiseMult with FIRST and SECOND operators
%0
logstat ('test138',s) ; % test assign, with coarse-only tasks in IxJ slice
%0
logstat ('test139',s) ; % merge sort, special cases
%0
logstat ('test72',t) ;  % several special cases
logstat ('test72',s) ;  % several special cases
%0
logstat ('test07b',t) ; % quick test GB_mex_assign
%0
logstat ('test09',t) ;  % duplicate I,J test of GB_mex_subassign
%0
logstat ('test83',t) ;  % GrB_assign with C_replace and empty J
%0
logstat ('test84',t) ;  % GrB_assign (row and column with C in CSR format)
logstat ('test84',s) ;  % GrB_assign (row and column with C in CSR format)
%0
logstat ('test85',t) ;  % GrB_transpose (1-by-n with typecasting)
logstat ('test85',s) ;  % GrB_transpose (1-by-n with typecasting)
%0.1
logstat ('test109',t) ; % terminal monoid with user-defined type
logstat ('test109',s);  % terminal monoid with user-defined type
%0.1
logstat ('test110',t) ; % binary search of M(:,j) in accum/mask
%0.1
logstat ('test131',t) ; % GrB_Matrix_clear
%0.1
logstat ('test132',t) ; % setElement
%0.1
logstat ('test98',t) ;  % GB_mex_mxm, typecast on the fly
logstat ('test98',s);   % GB_mex_mxm, typecast on the fly
%0.1
logstat ('test92',t) ;  % GB_subref (symbolic case)
%0.1
logstat ('test97',t) ;  % GB_mex_assign, scalar expansion and zombies
%0.1
logstat ('test01',t) ;  % error handling
logstat ('test01',s) ;  % error handling
%0.1
logstat ('test04',t) ;  % simple mask and transpose test
%0.1
logstat ('test05',t) ;  % quick setElement test, with typecasting
logstat ('test05',s);   % quick setElement test, with typecasting
%0.1
logstat ('test15',t) ;  % simple test of GB_mex_AxB
logstat ('test15',s) ;  % simple test of GB_mex_AxB
%0.1
logstat ('test78',t) ;  % quick test of hypersparse subref
%0.1
logstat ('test82',t) ;  % GrB_extract with index range (hypersparse)
%0.1
logstat ('test94',t) ;  % pagerank
logstat ('test94',s) ;  % pagerank
%0.2
logstat ('test126',t) ; % test GrB_reduce to vector on a very sparse matrix 
%0.2
logstat ('test03',t) ;  % random matrix tests
logstat ('test03',s) ;  % random matrix tests
%0.4
logstat ('test128',t) ; % eWiseMult, eWiseAdd, special cases
%0.4
logstat ('test02',t) ;  % matrix copy and dup tests
%0.4
logstat ('test17',t) ;  % quick test of GrB_*_extractElement
%0.5
logstat ('test108',t) ; % boolean monoids
%0.6
logstat ('test124',t) ; % GrB_extract, case 6
%0.6
logstat ('test101',t) ; % GrB_*_import and export
%0.6
logstat ('test26') ;    % quick test of GxB_select
%0.8
logstat ('test133',t) ; % %test mask operations (GB_masker)

%----------------------------------------
% tests taking 1 to 10 seconds:
%----------------------------------------

%1
logstat ('testc2(1)',t) ;  % complex tests (quick case)
%1
logstat ('test104',t) ; % export/import
%2
logstat ('test80',t) ;  % test GrB_mxm on all semirings (different matrix)
%2
logstat ('test130',t) ; % GrB_apply, hypersparse cases
%2
logstat ('test129',t) ; % test GxB_select (tril and nonzero, hypersparse)
%2
logstat ('test102',t);  % GB_AxB_flopcount
%2
logstat ('test12',t) ;  % Wathen finite-element matrices (short test)
%2
logstat ('test28',t) ;  % mxm with aliased inputs, C<C> = accum(C,C*C)
%2
logstat ('test29',t) ;  % reduce with zombies
%3
logstat ('test107') ;   % monoids with terminal values
%3
logstat ('test103',t) ; % GrB_transpose aliases
%3
logstat ('test93',t) ;  % pagerank
%3
logstat ('test135',t) ; % reduce to scalar
%2
logstat ('test100',t) ; % GB_mex_isequal
%4
logstat ('test11',t) ;  % exhaustive test of GrB_extractTuples
%5
logstat ('test106',t) ; % GxB_subassign with alias
%6
logstat ('test14',t) ;  % GrB_reduce
%6
logstat ('test69',t) ;  % assign and subassign with alias
%7
logstat ('test77',t) ;  % quick tests of GxB_kron
%5 + 4
logstat ('test19b',t) ; % GrB_assign, many pending operators (malloc debug off)
logstat ('test19b',s);  % GrB_assign, many pending operators (malloc debug off)

%----------------------------------------
% tests taking 10 to 200 seconds
%----------------------------------------

%5
logstat ('test00',s);   % GB_mex_mis (single threaded)
%10
logstat ('test76',t) ;  % GxB_resize
%12
logstat ('test88',t) ;  % hypersparse matrices with heap-based method
%13
logstat ('test54',t) ;  % assign and extract with begin:inc:end
%16
logstat ('test127',t) ; % test eWiseAdd, eWiseMult (all types and operators)
%17
logstat ('test99',t) ;  % GB_mex_transpose with explicit zeros in the Mask
%17
logstat ('test19',t) ;  % GxB_subassign, many pending operators
%18
logstat ('test23',t) ;  % quick test of GB_*_build
%20
logstat ('test125',t) ; % test GrB_mxm: row and column scaling
%33
logstat ('test53',t) ;  % quick test of GB_mex_Matrix_extract
%37
logstat ('test27',t) ;  % quick test of GxB_select (band)
%167
logstat ('test74',t) ;  % test GrB_mxm on all semirings

%----------------------------------------
% longer tests (200 seconds to 600 seconds)
%----------------------------------------

%330
logstat ('test90',t) ;  % test pre-compiled user-defined semirings

% Turn off malloc debugging
malloc_debugging = stat ;
if (malloc_debugging)
    debug_off
    fprintf ('[malloc debugging turned off]\n') ;
    f = fopen ('log.txt', 'a') ;
    fprintf (f, '[malloc debugging turned off]\n') ;
    fclose (f) ;
end

%79
logstat ('test134',t) ; % quick test of GxB_select

%80
logstat ('test10',t) ;  % GrB_apply

%92
logstat ('test16',t) ;  % user-defined complex operators

%96: only single-threaded is needed
logstat ('test21',s) ;  % quick test of GB_mex_subassign

%103
logstat ('test81',t) ;  % GrB_Matrix_extract with stride, range, backwards

%102
logstat ('test21b',t) ; % quick test of GB_mex_assign

%200
logstat ('test18',t) ;  % quick tests of GrB_eWiseAdd and eWiseMult

%292
logstat ('test75',t) ;  % test GrB_mxm A'*B on all semirings

%430
logstat ('test20',t) ;  % quick test of GB_mex_mxm on a few semirings

%-------------------------------------------------------------------------------
% The following tests are not required for statement coverage.  Some need
% other packages in SuiteSparse (CSparse, SSMULT, ssget).  By default, these
% tests are not run.  To install them, see test_other.m.  Timing is with malloc
% debugging turned off.

if (longtests)

% ------------------------ % ---- % ------------------------------
% test script              % time % description
% ------------------------ % ---- % ------------------------------

logstat ('test00',t) ;     %    8 % GB_mex_mis (multiple threads)
logstat ('test07',t) ;     %    0 % quick test GB_mex_subassign
logstat ('test07',s) ;     %    0 % quick test GB_mex_subassign
logstat ('test06',t) ;     %  532 % test GrB_mxm on all semirings
logstat ('test06(936)',t); %      % performance test GrB_mxm on all semirings
logstat ('test08',t) ;     %   35 % quick test GB_mex_subassign
logstat ('test08b',t) ;    %      % quick test GB_mex_assign
logstat ('test09b',t) ;    %      % duplicate I,J test of GB_mex_assign

logstat ('test12(0)',t) ;  %      % Wathen finite-element matrices (full test)
logstat ('test13',t) ;     %      % simple tests of GB_mex_transpose
logstat ('test18(1)',t) ;  %      % lengthy tests of GrB_eWiseAdd and eWiseMult

logstat ('test20(1)',t) ;  %      % test of GB_mex_mxm on all built-in semirings
logstat ('test21(1)',t) ;  %      % exhaustive test of GB_mex_subassign
logstat ('test22',t) ;     %      % quick test of GB_mex_transpose
logstat ('test23(1)',t) ;  %      % exhaustive test of GB_*_build
logstat ('test24',t) ;     %   42 % test of GrB_Matrix_reduce
logstat ('test24(1)',t) ;  %      % exhaustive test of GrB_Matrix_reduce
logstat ('test25',t) ;     %      % long test of GxB_select
logstat ('test26(1)',t) ;  %      % performance test of GxB_select (use ssget)

logstat ('test30') ;       %   11 % GB_mex_subassign, scalar expansion
logstat ('test30b') ;      %    9 % performance GB_mex_assign, scalar expansion
logstat ('test31',t) ;     %      % simple tests of GB_mex_transpose
logstat ('test32',t) ;     %      % quick GB_mex_mxm test
logstat ('test33',t) ;     %      % create a semiring
logstat ('test34',t) ;     %      % quick GB_mex_eWiseAdd_Matrix test
logstat ('test35') ;       %      % performance test for GrB_extractTuples
logstat ('test36') ;       %      % performance test for GB_mex_Matrix_subref
logstat ('test37') ;       %      % performance test for GrB_qsort1
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
logstat ('test55',t) ;     %      % GxB_subassign, dupl, MATLAB vs GraphBLAS
logstat ('test55b',t) ;    %      % GrB_assign, duplicates, MATLAB vs GraphBLAS
logstat ('test56',t) ;     %      % test GrB_*_build
logstat ('test57',t) ;     %      % test operator on large uint32 values
logstat ('test58(0)') ;    %      % longer GB_mex_eWiseAdd_Matrix performance
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

logstat ('test70',t) ;     %      % performance of triangle counting methods
logstat ('test71',t) ;     %      % performance of triangle counting methods
logstat ('test73',t) ;     %      % performance of C = A*B, with mask
logstat ('test79',t) ;     %      % run all in SuiteSparse Collection w/ test06

logstat ('test86',t) ;     %      % performance test of of GrB_Matrix_extract
logstat ('test87',t) ;     %      % performance test of GrB_mxm
logstat ('test89',t) ;     %      % performance test of complex A*B

logstat ('test91',t) ;     %      % test subref performance on dense vectors
logstat ('test93b',t) ;    %      % dpagerank and ipagerank
logstat ('test95',t) ;     %      % performance test for GrB_transpose
logstat ('test96',t) ;     %   16 % A*B using dot product

logstat ('test105',t) ;    %    2 % eWiseAdd for hypersparse

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
logstat ('test123',t) ;    %      % test MIS on large matrix

% tested via test16:
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
fprintf ('\ntestall: all tests passed, total time %0.2g hours\n', t / 3600) ;

