function test35
%TEST35 test GrB_*_extractTuples

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n test35 ---------------------- quick test of GrB_extractTuples\n') ;

Prob = ssget ('HB/west0067') ;
A = Prob.A ;
[I1, J1, X1] = find (A) ;
[I2, J2, X2] = GB_mex_extractTuples (A) ;
assert (isequal (I1, double (I2+1)))
assert (isequal (J1, double (J2+1)))
assert (isequal (X1, X2))

Prob = ssget (2662) ;
A = Prob.A ;

% add some dense columns
n = size (A,2) ;
J = randperm (n, 20) ;
A (:,J) = 77 ;

% warmup
[I1, J1, X1] = find (A) ;
tic
[I1, J1, X1] = find (A) ;
tm = toc ;
fprintf ('MATLAB [i,j,x]=find(A) time: %g\n', tm) ;

nthreads_max = 2*GB_mex_omp_max_threads ;
save = nthreads_get ;

t1 = 0 ;
for nthreads = [1 2 4 8 16 20 32 40 64 128 260 256]
    if (nthreads > nthreads_max)
        break ;
    end
    nthreads_set (nthreads) ;
    % warmup
    [I2, J2, X2] = GB_mex_extractTuples (A) ;
    % for timing
    [I2, J2, X2] = GB_mex_extractTuples (A) ;
    t = grbresults ;
    if (nthreads == 1)
        t1 = t ;
    end
    fprintf ('nthreads %3d speedup vs MATLAB: %10.4f  vs GrB: %10.4f\n', ...
        nthreads, tm/t, t1/t) ;
    assert (isequal (I1, double (I2+1)))
    assert (isequal (J1, double (J2+1)))
    assert (isequal (X1, X2))
end

nthreads_set (save) ;
fprintf ('\ntest35: all tests passed\n') ;

