% test GrB_extract
clear all
make
addpath ('~/ssget') ;
addpath ('spok') ;

for nth = [2 1]
    nthreads_set (nth) ;

    debug_on
    gb
    testc3
    testc4
    test53
    test81
    test82
    test86

    debug_off
    gb
    testc3
    testc4
    test53
    test81
    test82
    test86

end
