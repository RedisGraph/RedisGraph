% test GrB_assign and GxB_subassign
clear all
make
addpath ('~/ssget') ;
addpath ('spok') ;

% tests Method11c

for nth = [2 1]
    nthreads_set (nth,1) ;

    for d = 0 % [1 0]
        if (d)
            debug_on
        else
            debug_off
        end
        gb

        test19
        test19b
        test21
        test21b

    end
end
