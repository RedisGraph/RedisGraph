% test GrB_build
clear all
nthreads_set(2)
gb
test56
test23

for nthreads = [1 2]
    fprintf ('\n================================================================================\n') ;
    fprintf ('===================== nthreads: %d\n', nthreads) ;
    fprintf ('================================================================================\n') ;
    nthreads_set(nthreads) ;
    test42 ;
end

for nthreads = [1 2]
    fprintf ('\n================================================================================\n') ;
    fprintf ('===================== nthreads: %d\n', nthreads) ;
    fprintf ('================================================================================\n') ;
    nthreads_set(nthreads) ;
    test45 ;
end

