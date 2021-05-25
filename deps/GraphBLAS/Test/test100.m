function test100 
%TEST100 test GB_mex_isequal

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

[~, ~, ~, types, ~, ~] = GB_spec_opsall ;
types = types.all ;

rng ('default') ;

fprintf ('\ntest100: GB_mex_isequal\n') ;

n = 5 ;
m = 6 ;

B = sprand (n, n, 0.1) ;
A = sprand (n, n, 0.1) ;

r0 = isequal (A, B) ;
r1 = GB_mex_isequal (A, B) ;
assert (r0 == r1) 

B = sprand (n, n, 0.1) ;
A = sprand (m, n, 0.1) ;

r0 = isequal (A, B) ;
r1 = GB_mex_isequal (A, B) ;
assert (r0 == r1) 

r0 = isequal (A, A) ;
r1 = GB_mex_isequal (A, A) ;
assert (r0 == r1) 

A = sprand (5, 5, 0.1) ;
A = A+A' ;
r0 = isequal (A, A') ;
r1 = GB_mex_isequal (A, A') ;
assert (r0 == r1) 

for k = [false true]
    fprintf ('\nbuiltin_complex: %d ', k) ;
    GB_builtin_complex_set (k) ;

    % complex case:
    A = sprand (5, 5, 0.1) + 1i * sprand (5, 5, 0.1) ;
    B = sprand (5, 5, 0.1) + 1i * sprand (5, 5, 0.1) ;
    r0 = isequal (A, B) ;
    r1 = GB_mex_isequal (A, B) ;
    assert (r0 == r1) 

    r0 = isequal (A, A) ;
    r1 = GB_mex_isequal (A, A) ;
    assert (r0 == r1) 

    A = A+A.' ;
    r0 = isequal (A, A.') ;
    r1 = GB_mex_isequal (A, A.') ;
    assert (r0 == r1) 

    density = 0.5 ;
    scale = 100 ;
    ntypes = length (types) ;

    for k1 = 1:ntypes
        atype = types {k1} ;
        fprintf ('.') ;
        for am = [1 5]
            for an = [1 5]
                A = GB_spec_random (am,an,density,100,atype) ;

                r0 = isequal (A, A) ;
                r1 = GB_mex_isequal (A, A) ;
                assert (r0 == r1) 

                Amat = GB_mex_cast (full (A.matrix), atype) ;

                for k2 = 1:ntypes
                    bclas = types {k2} ;
                    for bm = [1 5]
                        for bn = [1 5]

                            B = GB_spec_random (bm,bn,density,100,bclas) ;

                            % r0 = isequal (A, B) ;
                            Bmat = GB_mex_cast (full (B.matrix), bclas) ;
                            r0 = isequal (Amat, Bmat) & ...
                                isequal (atype, bclas) & ...
                                    isequal (A.pattern, B.pattern) ;

                            r1 = GB_mex_isequal (A, B) ;
                            assert (r0 == r1) 

                        end
                    end
                end
            end
        end
    end
end

fprintf ('\ntest100: all tests passed\n') ;

