function test44
%TEST44 test qsort

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n------------------------------------- qsort tests\n') ;

rng ('default') ;

n = 20e6 ;

fprintf ('\n----------------------- qsort 1a\n') ;

I = int64 ((n/10)* rand (n,1)) ;

tic
Iout1 = sort (I) ;
t = toc ;

tic
Iout = GB_mex_qsort_1 (I) ;
t2 = toc ;

fprintf ('MATLAB: sort %g sec  qsort1a: %g  speedup: %g\n', t, t2, t/t2) ;

assert (isequal (Iout, Iout1))

fprintf ('\n----------------------- qsort 2a\n') ;

% qsort2a is not stable; it used only when I has unique values
I = int64 (randperm (n))' ;
J = int64 ((n/10)* rand (n,1)) ;
IJ = [I J] ;

tic
IJout = sortrows (IJ, 1) ;
t = toc ;

tic
[Iout, Jout] = GB_mex_qsort_2a (I, J) ;
t2 = toc ;

fprintf ('MATLAB: sortrows %g sec  qsort2a: %g  speedup: %g\n', t, t2, t/t2) ;

assert (isequal ([Iout Jout], IJout))

fprintf ('\n----------------------- qsort 2b\n') ;

I = int64 ((n/10)* rand (n,1)) ;
J = int64 (randperm (n))' ;
IJ = [I J] ;

tic
IJout = sortrows (IJ) ;
t = toc ;

tic
[Iout, Jout] = GB_mex_qsort_2b (I, J) ;
t2 = toc ;

fprintf ('MATLAB: sortrows %g sec  qsort2a: %g  speedup: %g\n', t, t2, t/t2) ;

assert (isequal ([Iout Jout], IJout));

fprintf ('\n----------------------- qsort 3\n') ;

I = int64 ((n/10)* rand (n,1)) ;
J = int64 ((n/10)* rand (n,1)) ;
K = int64 (randperm (n))' ;
IJK = [I J K] ;

tic
IJKout = sortrows (IJK) ;
t = toc ;

tic
[Iout, Jout, Kout] = GB_mex_qsort_3 (I, J, K) ;
t2 = toc ;

fprintf ('MATLAB: sortrows %g sec  qsort2a: %g  speedup: %g\n', t, t2, t/t2) ;

assert (isequal ([Iout Jout Kout], IJKout))

fprintf ('\ntest44: all tests passed\n') ;

