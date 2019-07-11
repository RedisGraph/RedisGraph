function test100 
%TEST100 test GB_mex_isequal

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[~, ~, ~, classes, ~, ~] = GB_spec_opsall ;

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
nclass = length (classes) ;

for k1 = 1:nclass
    aclas = classes {k1} ;
    fprintf ('.') ;
    for am = [1 5]
        for an = [1 5]
            A = GB_spec_random (am,an,density,100,aclas) ;

            r0 = isequal (A, A) ;
            r1 = GB_mex_isequal (A, A) ;
            assert (r0 == r1) 

            Amat = GB_mex_cast (full (A.matrix), aclas) ;

            for k2 = 1:nclass
                bclas = classes {k2} ;
                for bm = [1 5]
                    for bn = [1 5]

                        B = GB_spec_random (bm,bn,density,100,bclas) ;

                        % r0 = isequal (A, B) ;
                        Bmat = GB_mex_cast (full (B.matrix), bclas) ;
                        r0 = isequal (Amat, Bmat) & isequal (aclas, bclas) & isequal (A.pattern, B.pattern) ;

                        r1 = GB_mex_isequal (A, B) ;
                        assert (r0 == r1) 

                    end
                end
            end
        end
    end
end

fprintf ('\ntest100: all tests passed\n') ;

