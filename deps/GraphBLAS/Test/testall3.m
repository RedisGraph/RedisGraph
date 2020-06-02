%TESTALL3 run testall with different # of threads

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

clear all
make
grbinfo
for k = [1 2 4]
    nthreads_set (k) ;
    debug_off ;
    testall 
    debug_on
    testall 
end
