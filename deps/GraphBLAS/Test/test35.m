function test35
%TEST35 test GrB_*_extractTuples

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n ---------------------- quick test of GrB_extractTuples\n') ;

Prob = ssget ('HB/west0067') ;
A = Prob.A ;
[I1, J1, X1] = find (A) ;
[I2, J2, X2] = GB_mex_extractTuples (A) ;
assert (isequal (I1, double (I2+1)))
assert (isequal (J1, double (J2+1)))
assert (isequal (X1, X2))

Prob = ssget (2662) ;
A = Prob.A ;
tic
[I1, J1, X1] = find (A) ;
tm = toc ;
tic
[I2, J2, X2] = GB_mex_extractTuples (A) ;
toc
t = gbresults
assert (isequal (I1, double (I2+1)))
assert (isequal (J1, double (J2+1)))
assert (isequal (X1, X2))

fprintf ('GraphBLAS speedup over MATLAB; %g\n', tm/t) ;

fprintf ('\ntest35: all tests passed\n') ;

