% test GrB_extract

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

clear all
make
addpath ('~/ssget') ;
addpath ('spok') ;

for nth = [2 1]
    nthreads_set (nth) ;

    debug_on
    grbinfo
    testc3
    testc4
    test53
    test81
    test82
    test86

    debug_off
    grbinfo
    testc3
    testc4
    test53
    test81
    test82
    test86

end
