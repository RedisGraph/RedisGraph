function test139
%TEST139 merge sort, special cases

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('test139 --------------- merge sort, special cases\n') ;
rng ('default') ;

n = 1e6 ;
I = 42 * ones (n,1) ;
J = (1:n)' ;
K = 100 * ones (n,1) ;

I0 = int64 (I) ;
J0 = int64 (J) ;
K0 = int64 (K) ;

IJ1 = sortrows ([I0 J0]) ;
[a b] = GB_mex_msort_2 (I0, J0, 2) ;
assert (isequal (IJ1, [a b])) ;

IJ3 = sortrows ([I0 J0 K0]) ;
[a b c] = GB_mex_msort_3 (I0, J0, K0, 2) ;
assert (isequal (IJ3, [a b c])) ;

IJ1 = sortrows ([J0 I0]) ;
[a b] = GB_mex_msort_2 (J0, I0, 2) ;
assert (isequal (IJ1, [a b])) ;

a = GB_mex_msort_1 (I0, 1) ;
c = sort (I0) ;
assert (isequal (c, a)) ;

a = GB_mex_msort_1 (J0, 1) ;
c = sort (J0) ;
assert (isequal (c, a)) ;

I0 = int64 (randperm (10000, 5000)) ;
a = GB_mex_msort_1 (I0, 8) ;
c = sort (I0) ;
assert (isequal (c, a')) ;

for n = [10 100 1000 1e5 1e6]
    I0 = int64 (1000 * rand (n,1)) ;
    a = GB_mex_msort_1 (I0, 8) ;
    c = sort (I0) ;
    assert (isequal (c, a)) ;
end

for n = [10 100 1000 1e5 1e6]
    I0 = int64 (4 * ones (n,1)) ;
    a = GB_mex_msort_1 (I0, 8) ;
    c = sort (I0) ;
    assert (isequal (c, a)) ;
end

fprintf ('test139 --------------- all tests passed\n') ;
