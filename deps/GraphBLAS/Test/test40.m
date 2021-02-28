function test40
%TEST40 test GrB_Matrix_extractElement

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n ------ quick test of GrB_Matrix_extractElement\n') ;

Prob = ssget (936) ;
A = Prob.A ;
[m n] = size (A) ;

for i = double (1:500:m)
    for j = double (1:500:n)
        x1 = full (A (i,j)) ;
        x2 = GB_mex_Matrix_extractElement (A, uint64 (i-1), uint64 (j-1)) ;
        assert (isequal (x1,x2)) ;
    end
end

i2 = double (1:10:m) ;
j2 = double (1:10:n) ;
ni = length (i2) * length (j2) ;
I = zeros (ni, 1) ;
J = zeros (ni, 1) ;
X = zeros (ni, 1) ;

k = 0 ;
for i = i2
    for j = j2
        k = k + 1 ;
        I (k) = i ;
        J (k) = j ;
    end
end

% MATLAB loop 1
tic
for i = i2
    for j = j2
        x1 = full (A (i,j)) ;
    end
end
t1 = toc ;
fprintf ('MATLAB, scalar A(i,j): %g sec\n', t1) ;

% MATLAB loop 2 is about the same speed
tic
for k = 1:ni
    X (k) = full (A (I (k), J (k))) ;
end
t2 = toc ;
fprintf ('MATLAB, scalar A(i,j): %g sec (2nd loop)\n', t2) ;
t0 = min (t1, t2) ;

I = uint64 (I-1) ;
J = uint64 (J-1) ;
X1 = zeros (ni, 1) ;

% this is about 3x slower than MATLAB loop 1 and 2
tic
for k = 1:ni
    X1 (k) = GB_mex_Matrix_extractElement (A, I (k), J (k)) ;
end
t3 = toc ;
fprintf ('GrB     single A(i,j): %g sec speedup: %g\n', t3, t0/ t3) ;

% This is about 15x faster than MATLAB loop 1 and 2.
% the loop is internal in the mexFunction, so this code
% avoids the interpretive overhead.  It also avoids the
% malloc need to construct the GraphBLAS header for
% the shallow copy of A, and the malloc for the scalar result,
% for each iteration. Instead, those mallocs are done once.
tic
X2 = GB_mex_Matrix_extractElement (A, I, J) ;
t4 = toc ;
fprintf ('GrB     many   A(i,j): %g sec speedup: %g\n', t4, t0/ t4) ;

assert (isequal (X, X2)) ;
assert (isequal (X, X1)) ;


tic
[I,J,X] = find (A) ;
t7 = toc ;
fprintf ('MATLAB, [I,J,X] = find(A) %g sec\n', t7) ;

tic
[I2, J2, X2] = GB_mex_extractTuples (A) ;
t8 = toc ;
fprintf ('GrB     [I,J,X] = find(A) %g sec, speedup %g\n', t8, t7/t8) ;
assert (isequal (I, double (I2+1)))
assert (isequal (J, double (J2+1)))
assert (isequal (X, X2))

ni = length (I) ;

% MATLAB loop 3
tic
for k = 1:ni
    X (k) = full (A (I (k), J (k))) ;
end
t5 = toc ;
fprintf ('MATLAB, scalar A(i,j): %g sec (all entries)\n', t5) ;

tic
X2 = GB_mex_Matrix_extractElement (A, I2, J2) ;
t6 = toc ;
fprintf ('GrB     many   A(i,j): %g sec (all entries) speedup: %g\n', t6, t5/ t6) ;

assert (isequal (X, X2)) ;

%-------------------------------------------------------------------------------
% vector

V = A (:) ;

mn = length (V) ;
for i = double (1:500:mn)
    x1 = full (V (i)) ;
    x2 = GB_mex_Vector_extractElement (V, uint64 (i-1)) ;
    assert (isequal (x1,x2)) ;
end

tic
[I,J,X] = find (V) ;
t9 = toc ;
fprintf ('MATLAB, [I,J,X] = find(V) %g sec\n', t9) ;
ni = length (I) ;

tic
[I2, J2, X2] = GB_mex_extractTuples (V) ;
t8 = toc ;
fprintf ('GrB     [I,J,X] = find(V) %g sec, speedup %g\n', t8, t7/t8) ;
assert (isequal (I, double (I2+1)))
assert (isequal (J, double (J2+1)))
assert (isequal (X, X2))

% MATLAB vector loop
tic
for k = 1:ni
    X (k) = full (V (I (k))) ;
end
t10 = toc ;
fprintf ('MATLAB, scalar V(i): %g sec (all entries)\n', t10) ;

tic
X2 = GB_mex_Vector_extractElement (V, I2) ;
t6 = toc ;
fprintf ('GrB     many   A(i,j): %g sec (all entries) speedup: %g\n', t6, t5/ t6) ;

assert (isequal (X, X2)) ;

fprintf ('\ntest40: all tests passed\n') ;

