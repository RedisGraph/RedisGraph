function test42
%TEST42 test GrB_Matrix_build

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n----------------------- performance tests for GrB_Matrix_build\n') ;

Prob = ssget ('HB/west0067')
A = Prob.A ;
[m n] = size (A) ;

[i j x] = find (Prob.A) ;
i = uint64 (i-1) ;
j = uint64 (j-1) ;
T = sparse (double (i+1), double (j+1), x) ;
assert (isequal (A,T)) ;

S = GB_mex_Matrix_build (i,j,x) ;
S = S.matrix ;
assert (isequal (A,S)) ;
assert (spok (S) == 1) ;

nz = nnz (A) ;
p = randperm (nz) ;
i = i(p) ;
j = j(p) ;
x = x(p) ;

S = GB_mex_Matrix_build (i,j,x) ;
S = S.matrix ;
assert (isequal (A,(S')'))
assert (isequal (A,S)) ;
assert (spok (S) == 1) ;

% duplicates
rng ('default') ;
i2 = floor (rand (100,1) * n) + 1 ;
j2 = floor (rand (100,1) * n) + 1 ;
x2 = rand (100,1) ;
i = [i ; uint64(i2-1)] ;
j = [j ; uint64(j2-1)] ;
x = [x ; x2] ;
T = sparse (double (i+1), double (j+1), x) ;

S = GB_mex_Matrix_build (i,j,x) ;
S = S.matrix ;
assert (isequal (spones (S), spones (T)))
assert (norm (S-T,1) == 0) ;
assert (spok (T) == 1) ;

% for col = 1:n
%     S (:,col)
%     T (:,col)
%     norm (S (:,col) - T (:,col), 1)
%     % pause
% end

fprintf ('matrix from collection, no sorting:\n') ;
Prob = ssget (939)
A = Prob.A ;
[m n] = size (A) ;
[i j x] = find (Prob.A) ;
i = uint64 (i-1) ;
j = uint64 (j-1) ;
i1 = double (i+1) ;
j1 = double (j+1) ;
fprintf ('MATLAB:\n') ;
tic
T = sparse (i1, j1, x) ;
toc
assert (isequal (A,T))

fprintf ('GrB:\n') ;
tic
S = GB_mex_Matrix_build (i,j,x) ;
S = S.matrix ;
toc
assert (isequal (A,S))
assert (spok (S) == 1) ;

try 
    fprintf ('Csparse:\n') ;
    tic
    W = cs_sparse (i1,j1,x) ;
    toc
    ok = isequal (A,W) && (spok (W) == 1) ;
catch
    % CSparse not available
    ok = true ;
end
assert (ok) ;

fprintf ('sparse2:\n') ;
try
    tic
    Y = sparse2 (i1,j1,x) ;
    toc
    ok = (isequal (A,Y)) && assert (spok (Y) == 1) ;
catch
    % CHOLMOD not available
    ok = true ;
end
assert (ok) ;

fprintf ('\nrandom matrix, with duplicates:\n') ;
i2 = floor (rand (1000000,1) * n) + 1 ;
j2 = floor (rand (1000000,1) * n) + 1 ;
x2 = rand (1000000,1) ;
i = [i ; uint64(i2-1)] ;
j = [j ; uint64(j2-1)] ;
x = [x ; x2] ;
i1 = double (i+1) ;
j1 = double (j+1) ;

fprintf ('MATLAB:\n') ;
tic
T = sparse (i1, j1, x) ;
toc

fprintf ('GrB:\n') ;
tic
S = GB_mex_Matrix_build (i,j,x) ;
S = S.matrix ;
toc
% norm (T-S,1)
assert (isequal (T,S))
assert (spok (S) == 1) ;

try
    fprintf ('Csparse:\n') ;
    tic
    W = cs_sparse (i1,j1,x) ;
    toc
    ok = isequal (T,W) && (spok (W) == 1) ;
catch
    % CSparse not available
    ok = true ;
end
assert (ok) ;

fprintf ('sparse2:\n') ;
try
    tic
    Y = sparse2 (i1,j1,x) ;
    toc
    % norm (T-Y,1)
    ok = (isequal (T,Y)) && assert (spok (Y) == 1) ;
catch
    % CHOLMOD not available
    ok = true ;
end
assert (ok) ;

fprintf ('\nsame random matrix, but presorted:\n') ;
[ignore,p] = sortrows ([j i x]) ;
i = i (p) ;
j = j (p) ;
x = x (p) ;
i1 = i1 (p) ;
j1 = j1 (p) ;

fprintf ('MATLAB:\n') ;
tic
T = sparse (i1, j1, x) ;
toc

fprintf ('GrB:\n') ;
tic
S = GB_mex_Matrix_build (i,j,x) ;
S = S.matrix ;
toc
% norm (T-S,1)
assert (isequal (T,S))
assert (spok (S) == 1) ;

try
    fprintf ('CSparse:\n') ;
    tic
    W = cs_sparse (i1,j1,x) ;
    toc
    ok = isequal (T,W) && (spok (W) == 1) ;
catch
    % CSparse not available
    ok = true ;
end
assert (ok) ;

fprintf ('sparse2:\n') ;
try
    tic
    Y = sparse2 (i1,j1,x) ;
    toc
    % norm (T-Y,1)
    ok = (isequal (T,Y)) && assert (spok (Y) == 1) ;
catch
    % CHOLMOD not available
    ok = true ;
end
assert (ok) ;

fprintf ('\ntest42: all tests passed\n') ;

