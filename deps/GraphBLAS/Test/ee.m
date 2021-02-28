%EE eWiseMult and eWiseAdd performance tests

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

clear all
addpath ('~/ssget') ;
addpath ('spok') ;

for threads = 1:4
    nthreads_set (threads) ;
    grbinfo
    test68  % eWiseMult performance
    test58(0)  % eWiseAdd performance
    test61  % eWiseMult performance
    test39  % eWiseAdd performance
end

