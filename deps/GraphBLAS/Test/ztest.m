% test zombie deletion

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

clear all
nthreads_set(1)
grbinfo
test29
test97

nthreads_set(2)
test29
test97
