function test66
%TEST66 test GrB_reduce

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n---- quick test for GrB_reduce_to_scalar and vector\n') ;

rng ('default') ;
A = sparse (rand (4,3))
x = full (sum (sum (A))) + 1.3
c = GB_mex_reduce_to_scalar (1.3, 'plus', 'plus', A)
assert (isequal (x,c))

tic
x = full (sum (sum (A))) ;
toc
tic
y = GB_mex_reduce_to_scalar (0, '', 'plus', A) ;
toc
assert (norm(x-y) < nnz (A) * eps * norm(x))

tic
x = full (sum (A (:))) ;
toc
tic
y = GB_mex_reduce_to_scalar (0, '', 'plus', A) ;
toc
assert (isequal (x,y))


% reduce to vector
y = sparse (4,1) ;
y = GB_mex_reduce_to_vector (y, [ ], '', 'plus', A) ;
assert (isequal (y.matrix, sum (A')'))

clear d
d.inp0 = 'tran' ;
y = sparse (3,1) ;
y = GB_mex_reduce_to_vector (y, [ ], '', 'plus', A, d) ;
assert (isequal (y.matrix, sum (A)'))

A = sprand (3e6, 3e6, 2e-6) ;
n = size (A,1) ;
yin = sparse (rand (n,1)) ;

% sum across the rows
fprintf ('row sum:\n') ;
tic
y2 = yin + (sum (A,2)) ;
t1 = toc ;
tic
y = GB_mex_reduce_to_vector (yin, [ ], 'plus', 'plus', A) ;
t2 = toc ;
fprintf ('MATLAB: %g GraphBLAS %g speedup %g\n', t1, t2, t1/t2) ;
assert (isequal (y.matrix, y2))

% sum down the columns
fprintf ('col sum:\n') ;
yinrow = yin' ;
tic
y2 = yinrow + (sum (A,1)) ;
t1 = toc ;
tic
y = GB_mex_reduce_to_vector (yin, [ ], 'plus', 'plus', A, d) ;
t2 = toc ;
fprintf ('MATLAB: %g GraphBLAS %g speedup %g\n', t1, t2, t1/t2) ;
assert (isequal (y.matrix, y2'))

% reduce to scalar
fprintf ('to scalar:\n') ;
tic
x = full (sum (sum (A))) ;
t1 = toc ;
tic
y = GB_mex_reduce_to_scalar (0, '', 'plus', A) ;
t2 = toc ;
fprintf ('MATLAB: %g GraphBLAS %g speedup %g\n', t1, t2, t1/t2) ;
assert (norm(x-y) < nnz (A) * eps * norm(x))

fprintf ('\ntest66: all tests passed\n') ;

