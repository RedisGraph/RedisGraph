% test mxm

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

clear all
make
addpath ('~/ssget') ;
addpath ('spok') ;
% debug_on
nthreads_set (2) ;
grbinfo


%
test06
test16
test20
test28
test32
test49
test59
test72
test73
test74
test75
test87
test88
test98
test48
testall
testca
testperf
