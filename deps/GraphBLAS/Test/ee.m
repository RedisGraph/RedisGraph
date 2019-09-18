%EE eWiseMult and eWiseAdd performance tests
clear all
addpath ('~/ssget') ;
addpath ('spok') ;

for threads = 1:4
    nthreads_set (threads) ;
    gb
    test68  % eWiseMult performance
    test58(0)  % eWiseAdd performance
    test61  % eWiseMult performance
    test39  % eWiseAdd performance
end

