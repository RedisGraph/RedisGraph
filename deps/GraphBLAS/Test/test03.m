function test03
%TEST03 test GB_check functions

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[mult_ops unary_ops add_ops classes semirings] = GB_spec_opsall ;

rng ('default') ;

for k = 1:length (classes)
    aclass = classes {k} ;
    for is_hyper = 0:1
        for is_csc = 0:1
            A = GB_spec_random (10,30,0.2,100,aclass, is_csc, is_hyper) ;
            GB_mex_dump (A,2) ;
        end
    end
    for hyper_ratio = -0.1:0.1:0.4
        A = GB_spec_random (10,30,0.02,100,aclass, is_csc, [ ], hyper_ratio) ;
        GB_mex_dump (A,2) ;
    end
end

for is_hyper = 0:1
    for is_csc = 0:1
        A = GB_spec_random (100,2,0.5,100,'int8', is_csc, is_hyper) ;
        GB_mex_dump (A,2) ;
    end
end

A = GB_mex_random (10, 30, 15, 1, 1, 0, 0, 0) ;
GB_mex_dump (A,2) ;
A = GB_mex_random (10, 30, 15, 1, 1, 0, 0, 1) ;
GB_mex_dump (A,2) ;
A = GB_mex_random (10, 30, 15, 1, 1, 1, 0, 1) ;
GB_mex_dump (A,2) ;
A = GB_mex_random (10, 30, 15, 1, 1, 1, 0, 0) ;
GB_mex_dump (A,2) ;
A = GB_mex_random (10, 30, 15, 1, 1, 1, 1, 1) ;
GB_mex_dump (A,2) ;


A = GB_mex_random (3, 3, 5, 0, 1, 1, 1, 3) 
GB_mex_dump (A) 

fprintf ('\ntest03: all object check tests passed\n') ;

