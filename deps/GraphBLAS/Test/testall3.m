
clear all
make
gb
for k = [1 2 4]
    nthreads_set (k) ;
    debug_off ;
    testall 
    debug_on
    testall 
end
