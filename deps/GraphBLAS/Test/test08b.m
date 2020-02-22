function test08b
%TEST08B test GrB_assign

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
C = sparse (rand (5,4)) ;
A = 100 * sparse (magic (2)) ;

I = [2 3] ;
J = [3 4] ;

C2 = C ;
C2 (I,J) = A  ;

% full (C)
% full (C2)

I0 = uint64 (I-1) ;
J0 = uint64 (J-1) ;

C3 = GB_mex_assign (C, [ ], '', A, I0, J0, [ ]) ;
% C3.matrix
% full (C3.matrix)
assert (isequal (C3.matrix, C2))


C = sparse ([1 2 ;
             0 3]) ;
A = sparse (11) ;
I0 = uint64 (1) ;
J0 = uint64 (0) ;
C3 = GB_mex_assign (C, [ ], '', A, I0, J0, [ ]) ;
C2 = C ;
C2 (2,1) = 11 ;


C = sparse (rand (4)) ;
A = sparse (0) ;
C2 = C ;
C2 (1,1) = A ;
C3 = GB_mex_assign (C, [ ], '', A, uint64(0), uint64(0), [ ]) ;
assert (isequal (C3.matrix, C2))

w = load ('../Demo/Matrix/west0067') ;
A = sparse (w (:,1)+1, w (:,2)+1, w (:,3)) ;

[m n] = size (A) ;

for trial = 1:1000

    % fprintf ('.') ;

    ni = max (1, floor (m * rand (1)))  ;
    nj = max (1, floor (n * rand (1)))  ;
    % ni = 3 ;
    % nj = 3 ;
    I = randperm (m) ;
    I = I (1:ni) ;
    J = randperm (n) ;
    J = J (1:nj) ;

    I0 = uint64 (I-1) ;
    J0 = uint64 (J-1) ;

    B = sprandn (ni, nj, 0.1) ;

    C1 = A ;
    C1 (I,J) = B ;

    C2 = GB_mex_assign (A, [ ], '', B, I0, J0, [ ]) ;

    assert (isequal (C2.matrix, C1)) ;

end

fprintf ('\ntest08b: all tests passed\n') ;


