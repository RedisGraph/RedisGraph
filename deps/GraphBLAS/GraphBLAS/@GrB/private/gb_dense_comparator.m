function C = gb_dense_comparator (A, op, B)
%GB_DENSE_COMPARATOR compare two matrices, first expanding them to full.
% The pattern of C is a full matrix.  A and B must first be expanded to to
% a full matrix with explicit zeros.  For example, with A <= B for two
% matrices A and B:
%
%     in A        in B        A(i,j) <= B (i,j)    true or false
%     not in A    in B        0 <= B(i,j)          true or false
%     in A        not in B    A(i,j) <= 0          true or false
%     not in A    not in B    0 <= 0               true, in C

if (~GrB.isfull (A))
    A = full (A) ;
end

if (~GrB.isfull (B))
    B = full (B) ;
end

C = GrB.emult (A, op, B) ;

