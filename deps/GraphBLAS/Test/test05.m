function test05
%TEST05 test GrB_*_setElement

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

A = sparse (5,5) ;
A (2,2) = 42 ;

I = [1 2 3 2]' ;
J = [1 2 3 2]' ;

I0 = uint64 (I-1) ;
J0 = uint64 (I-1) ;
X = [4 5 6 3]' ;


A1 = GB_mex_setElement (A, I0, J0, int8(X))  ;

% A1_matrix = A1.matrix

B = A ;
for k = 1:length(I)
    B (I (k), J (k)) = X (k) ;
end

% B

assert (isequal (A1.matrix, B)) 

A2.matrix = A ;

for A_is_hyper = 0:1
    for A_is_csc = 0:1
        A2.is_csc   = A_is_csc ;
        A2.is_hyper = A_is_hyper ;

        A3 = GB_mex_setElement (A2, I0, J0, int8(X))  ;
        assert (isequal (A3.matrix, B)) 

    end
end

fprintf ('\ntest05: all tests passed\n') ;

