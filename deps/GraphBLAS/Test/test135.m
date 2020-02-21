function test135
%TEST135 reduce-to-scalar, built-in monoids with terminal values

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('test135: reduce to scalar\n') ;

rng ('default') ;

n = 10e6 ;

[save_nthreads, save_chunk] = nthreads_get ;
nthreads_max = feature ('numcores')

%-------------------------------------------------------------------------------
fprintf ('================== int8 min:\n') ;

X = rand (n,1) ;
X = (256 * X) - 128 ;
X = int8 (X) ;
s = int8 (inf) ;
tic
c0 = min (X) ;
tm = toc ;
fprintf ('MATLAB: %g sec\n', tm) ;

A.matrix = sparse (double (X)) ;
A.pattern = logical (spones (X)) ;
A.class = 'int8' ;

nthreads_set (1,1) ;
c1 = GB_mex_reduce_to_scalar (s, [ ], 'min', A) ;
assert (c1 == c0) ;
t1 = grbresults ;
fprintf ('1 thread  %g sec\n', t1) ;

nthreads_set (nthreads_max,1) ;
c2 = GB_mex_reduce_to_scalar (s, [ ], 'min', A) ;
assert (c2 == c0) ;
t2 = grbresults ;
fprintf ('%d threads %g sec\n', nthreads_max, t2) ;

%-------------------------------------------------------------------------------
fprintf ('================== double min:\n') ;

X = sparse (rand (n,1)) ;

A.matrix = X ;
A.pattern = logical (spones (X)) ;
A.class = 'double' ;

tic
c0 = min (X) ;
tm = toc ;
fprintf ('MATLAB: %g sec\n', tm) ;

s = double (inf) ;

nthreads_set (1,1) ;
c1 = GB_mex_reduce_to_scalar (s, [ ], 'min', A) ;
assert (c1 == c0) ;
t1 = grbresults ;
fprintf ('1 thread  %g sec\n', t1) ;

nthreads_set (nthreads_max,1) ;
c2 = GB_mex_reduce_to_scalar (s, [ ], 'min', A) ;
assert (c2 == c0) ;
t2 = grbresults ;
fprintf ('%d threads %g sec\n', nthreads_max, t2) ;

%-------------------------------------------------------------------------------
fprintf ('================== double sum:\n') ;

X = rand (n,1) ;
tic
c0 = sum (X) ;
tm = toc ;
fprintf ('MATLAB: %g sec (full)\n', tm) ;

X = sparse (X) ;

A.matrix = X ;
A.pattern = logical (spones (X)) ;
A.class = 'double' ;

tic
c0 = full (sum (X)) ;
tm = toc ;
fprintf ('MATLAB: %g sec (sparse)\n', tm) ;

s = double (inf) ;

nthreads_set (1,1) ;
c1 = GB_mex_reduce_to_scalar (s, [ ], 'plus', A) ;
assert (norm (c1 - c0) / norm (c0) < 1e-12) ;
t1 = grbresults ;
fprintf ('1 thread  %g sec\n', t1) ;

nthreads_set (nthreads_max,1) ;
c2 = GB_mex_reduce_to_scalar (s, [ ], 'plus', A) ;
assert (norm (c2 - c0) / norm (c0) < 1e-12) ;
t2 = grbresults ;
fprintf ('%d threads %g sec\n', nthreads_max, t2) ;

%-------------------------------------------------------------------------------
nthreads_set (save_nthreads, save_chunk) ;
fprintf ('test135: all tests passed\n') ;
