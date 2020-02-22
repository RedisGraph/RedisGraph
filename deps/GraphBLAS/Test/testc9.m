function testc9
%TESTC9 test complex extractTuples

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

seed = 1 ;
for m = [1 5 10 100]
    for n = [1 5 10 100]
        for trial = 1:100
            A = GB_mex_random (m, n, 10*(m+n), 1, seed) ; seed = seed + 1 ;

            [I  J  X ] = find (A) ;
            X = complex (X) ;

            [I0 J0 X0] = GB_mex_extractTuples (A) ;

            I1 = double (I0+1) ;
            J1 = double (J0+1) ;

            assert (isequal (I (:), I1 (:))) ;
            assert (isequal (J (:), J1 (:))) ;
            assert (isequal (X (:), X0 (:))) ;

        end
    end
end

fprintf ('testc9: all complex extractTuples tests passed\n') ;

