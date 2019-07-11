% test GrB_reduce to vector and scalar
clear all
make

for k = [4 2 1]

    nthreads_set (k) ;

    debug_off
    stat

    fprintf ('\n=============== GrB_reduce to scalar tests: nthreads %d\n', k) ;
    test29
    fprintf ('\n=============== GrB_reduce to vector tests: nthreads %d\n', k) ;
    test66
    test95
    test14
    test24(0)

    debug_on
    stat

    fprintf ('\n=============== GrB_reduce to scalar tests: nthreads %d\n', k) ;
    test29
    fprintf ('\n=============== GrB_reduce to vector tests: nthreads %d\n', k) ;
    test66
    test95
    test14
    test24(0)

end

test107
