function test69
%TEST69 test GrB_assign with aliased inputs, C<C>(:,:) = accum(C(:,:),C)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;

semiring.multiply = 'times' ;
semiring.add = 'plus' ;
semiring.class = 'double' ;

seed = 1 ;
for m = [1 5 10 100]
    for n = [1 5 10 100]
        for trial = 1:30
            A = GB_mex_random (m, n, 10*n, 0, seed) ; seed = seed + 1 ;
            C = GB_mex_random (m, n, 10*n, 0, seed) ; seed = seed + 1 ;

            C1 = GB_mex_assign_alias (C, 'plus', [ ], [ ], [ ]) ;
            C2 = GB_mex_assign (C, [ ], 'plus', C, [ ], [ ], [ ], 0) ;
            assert (isequal (C1, C2)) ;

            I = uint64 (randperm (m) - 1) ;
            J = uint64 (randperm (n) - 1) ;

            C1 = GB_mex_assign_alias (C, 'plus', I, J, [ ]) ;
            C2 = GB_mex_assign (C, [ ], 'plus', C, I, J, [ ], 0) ;
            assert (isequal (C1, C2)) ;

            C1 = GB_mex_subassign_alias (C, 'plus', [ ]) ;
            C2 = GB_mex_subassign (C, C, 'plus', C, [ ], [ ], [ ]) ;
            assert (isequal (C1, C2)) ;
        end
    end
end

fprintf ('test69: assign alias tests passed\n') ;

