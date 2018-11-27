function test87
%TEST87 performance test of GrB_mxm

rng ('default') ;

%-------------------------------------------------------------------------------

fprintf ('\n--------------------------------------------------\n') ;

k = 30e6
fprintf ('building random sparse matrix, %d by %d\n', k,2) ;
A = sprandn (k, 2, 0.01) ;
B = sprandn (k, 2, 0.01) ;

fprintf ('MATLAB:\n') ;
tic
C = A'*B ;
toc
tm = toc ;

fprintf ('GrB AdotB:\n') ;
tic
C2 = GB_mex_AdotB (A,B) ;
toc

fprintf ('GrB (A'')*B:\n') ;
tic
C3 = GB_mex_AxB (A',B) ;
toc

fprintf ('GrB A''*B native:\n') ;
tic
C4 = GB_mex_AxB (A,B, true) ;
toc
[tg method] = gbresults ;

assert (isequal (C, C2)) ;
assert (isequal (C, C3)) ;
assert (isequal (C, C4)) ;

fprintf ('MATLAB: %10.4f  GB:auto: %10.4f(%s) speedup %10.4f\n', ...
    tm, tg, method (1), tm/tg) ;

%-------------------------------------------------------------------------------
fprintf ('\n--------------------------------------------------\n') ;

k = 30e6
m = 100
fprintf ('building random sparse matrix, %d by %d\n', k,m) ;
A = sprandn (k, 2, 0.01) ;
B = sprandn (k, m, 0.01) ;

fprintf ('MATLAB:\n') ;
tic
C = A'*B ;
toc
tm = toc ;

fprintf ('GrB AdotB:\n') ;
tic
C2 = GB_mex_AdotB (A,B) ;
toc

fprintf ('GrB (A'')*B:\n') ;
tic
C3 = GB_mex_AxB (A',B) ;
toc
[tg1 method1] = gbresults ;
fprintf ('just A*B %g method %s (both A and B non-hypersparse)\n', tg1, method1) ;

% this is slower than GB_mex_AxB (A',B) even though it uses the
% same method, because the MATLAB A' above is non-hypersparse,
% but the internal AT=A' in GB_AxB_meta is hypersparse.

fprintf ('GrB A''*B native (AT becomes hypersparse):\n') ;
tic
C4 = GB_mex_AxB (A,B, true) ;
toc
[tg method] = gbresults ;

fprintf ('MATLAB: %10.4f  GB:auto: %10.4f(%s) speedup %10.4f\n', ...
    tm, tg, method (1), tm/tg) ;

assert (isequal (C, C2)) ;
assert (isequal (C, C3)) ;
assert (isequal (C, C4)) ;

%-------------------------------------------------------------------------------
fprintf ('\n--------------------------------------------------\n') ;

fprintf ('matrix A above, transposed:\n') ;

tic
AT1 = A' ;
toc
tm = toc ;

[mm nn] = size (AT1) ;
S = sparse (mm,nn) ;

tic
AT2 = GB_mex_transpose (S, [ ], [ ], A)
toc
[tg method] = gbresults ;

assert (isequal (AT1, AT2.matrix)) ;

fprintf ('size of AT: %d %d\n', mm, nn) ;
fprintf ('MATLAB transpose %g GB %g speedup %g\n', tm, tg, tm/tg) ;

fprintf ('GrB (AT)*B:\n') ;
tic
C3 = GB_mex_AxB (AT1,B) ;
toc
[tg1 method1] = gbresults ;
fprintf ('just A*B %g method %s\n', tg1, method1) ;

%-------------------------------------------------------------------------------
fprintf ('\n--------------------------------------------------\n') ;

fprintf ('\nA''*x where A is big and x is a dense vector\n') ;
Prob = ssget (2662) ;
A = Prob.A ;
n = size (A, 1) ;
x = sparse (rand (n,1)) ;
z = full (x) ;

fprintf ('MATLAB: x full:\n') ;
tic
y0 = A'*z ;
toc

fprintf ('MATLAB: x sparse:\n') ;
tic
y1 = A'*x ;
toc
tm = toc ;

fprintf ('GrB AdotB:\n') ;
tic
y2 = GB_mex_AdotB (A,x) ;
toc

fprintf ('GrB A''xB auto select:\n') ;
tic
y3 = GB_mex_AxB (A,x, true) ;
toc
[tg method] = gbresults ;

fprintf ('GrB (A'')xB outer:\n') ;
tic
y3 = GB_mex_AxB (A',x) ;
toc

assert (isequal (y1, sparse (y0))) ;
assert (isequal (y1, y2)) ;
assert (isequal (y1, y3)) ;

fprintf ('MATLAB: %10.4f  GB:auto: %10.4f(%s) speedup %10.4f\n', ...
    tm, tg, method (1), tm/tg) ;

%-------------------------------------------------------------------------------
fprintf ('\n--------------------------------------------------\n') ;

fprintf ('\nx''A where A is big and x is a dense vector\n') ;

fprintf ('MATLAB: x full:\n') ;
tic
y0 = z'*A ;
toc

fprintf ('MATLAB: x sparse:\n') ;
tic
y1 = x'*A ;
toc
tm = toc ;

fprintf ('GrB AdotB:\n') ;
tic
y2 = GB_mex_AdotB (x,A) ;
toc

fprintf ('GrB A''xB auto select:\n') ;
tic
y3 = GB_mex_AxB (x, A, true) ;
toc
[tg method] = gbresults ;

fprintf ('GrB (A''B outer:\n') ;
tic
y3 = GB_mex_AxB (x', A) ;
toc

assert (isequal (y1, sparse (y0))) ;
assert (isequal (y1, y2)) ;
assert (isequal (y1, y3)) ;

fprintf ('MATLAB: %10.4f  GB:auto: %10.4f(%s) speedup %10.4f\n', ...
    tm, tg, method (1), tm/tg) ;

%-------------------------------------------------------------------------------
fprintf ('\n--------------------------------------------------\n') ;
fprintf ('\nA*x where A is big and x is a dense vector\n') ;

fprintf ('MATLAB: x full:\n') ;
tic
y0 = A*z ;
toc

fprintf ('MATLAB: x sparse:\n') ;
tic
y1 = A*x ;
toc
tm = toc ;

fprintf ('GrB AxB:\n') ;
tic
y3 = GB_mex_AxB (A, x, false) ;
toc
[tg method] = gbresults ;

assert (isequal (y1, sparse (y0))) ;
assert (isequal (y1, y3)) ;

fprintf ('MATLAB: %10.4f  GB:auto: %10.4f(%s) speedup %10.4f\n', ...
    tm, tg, method (1), tm/tg) ;

%-------------------------------------------------------------------------------
fprintf ('\n--------------------------------------------------\n') ;

fprintf ('\nA''*x where A is big and x is a very sparse vector\n') ;
x = sprandn (n,1, 0.0001) ;

fprintf ('MATLAB: x sparse:\n') ;
tic
y1 = A'*x ;
toc
tm = toc ;

fprintf ('GrB AdotB:\n') ;
tic
y2 = GB_mex_AdotB (A,x) ;
toc

fprintf ('GrB A''xB auto select:\n') ;
tic
y3 = GB_mex_AxB (A,x, true) ;
toc
[tg method] = gbresults ;

fprintf ('GrB (A'')xB outer:\n') ;
tic
y3 = GB_mex_AxB (A',x) ;
toc

assert (isequal (y1, y2)) ;
assert (isequal (y1, y3)) ;

fprintf ('MATLAB: %10.4f  GB:auto: %10.4f(%s) speedup %10.4f\n', ...
    tm, tg, method (1), tm/tg) ;

fprintf ('\ntest87: all tests passed\n') ;

