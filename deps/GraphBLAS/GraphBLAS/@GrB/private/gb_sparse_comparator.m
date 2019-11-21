function C = gb_sparse_comparator (A, op, B)
%GB_SPARSE_COMPARATOR compare two sparse matrices.
% The pattern of C is the set union of A and B.  A and B must first be
% expanded to include explicit zeros in the set union of A and B.  For
% example, with A < B for two matrices A and B:
%
%     in A        in B        A(i,j) < B (i,j)    true or false
%     not in A    in B        0 < B(i,j)          true or false
%     in A        not in B    A(i,j) < 0          true or false
%     not in A    not in B    0 < 0               false, not in C
%
% expand A and B to the set union of A and B, with explicit zeros.
% The type of the '1st' operator is the type of the first argument of
% GrB.eadd, so the 2nd argument can be boolean to save space.

A0 = GrB.eadd ('1st', A, GrB.expand (false, B)) ;
B0 = GrB.eadd ('1st', B, GrB.expand (false, A)) ;
C = GrB.eadd (A0, op, B0) ;

