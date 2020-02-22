function C = le (A, B)
%A <= B Less than or equal to.
% C = (A <= B) is an element-by-element comparison of A and B.  One or
% both may be scalars.  Otherwise, A and B must have the same size.
%
% The input matrices may be either GraphBLAS and/or MATLAB matrices, in
% any combination.  C is returned as a GraphBLAS matrix.
%
% See also GrB/lt, GrB/gt, GrB/ge, GrB/ne, GrB/eq.

% The pattern of C depends on the type of inputs:
% A scalar, B scalar:  C is scalar.
% A scalar, B matrix:  C is full if A<=0, otherwise C is a subset of B.
% B scalar, A matrix:  C is full if B>=0, otherwise C is a subset of A.
% A matrix, B matrix:  C is full.
% Zeroes are then dropped from C after it is computed.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (isscalar (A))
    if (isscalar (B))
        % both A and B are scalars.  C is full.
        C = gb_dense_comparator (A, '<=', B) ;
    else
        % A is a scalar, B is a matrix
        if (gb_get_scalar (A) <= 0)
            % since a <= 0, entries not present in B result in a true
            % value, so the result is dense.  Expand A to a dense matrix.
            [m, n] = size (B) ;
            % A (1:m,1:n) = A and cast to the type of B
            A = GrB.subassign (GrB (m, n, GrB.type (B)), A) ;
            if (~GrB.isfull (B))
                B = full (B) ;
            end
            C = GrB.emult (A, '<=', B) ;
        else
            % since a > 0, entries not present in B result in a false
            % value, so the result is a sparse subset of B.  select all
            % entries in B >= a, then convert to true.
            C = GrB.apply ('1.logical', GrB.select (B, '>=', A)) ;
        end
    end
else
    if (isscalar (B))
        % A is a matrix, B is a scalar
        if (gb_get_scalar (B) >= 0)
            % since b >= 0, entries not present in A result in a true
            % value, so the result is dense.  Expand B to a dense matrix.
            [m, n] = size (A) ;
            % B (1:m,1:n) = B and cast to the type of A
            B = GrB.subassign (GrB (m, n, GrB.type (A)), B) ;
            if (~GrB.isfull (A))
                A = full (A) ;
            end
            C = GrB.emult (A, '<=', B) ;
        else
            % since b < 0, entries not present in A result in a false
            % value, so the result is a sparse subset of A.  select all
            % entries in A <= b, then convert to true.
            C = GrB.apply ('1.logical', GrB.select (A, '<=', B)) ;
        end
    else
        % both A and B are matrices.  C is full.
        C = gb_dense_comparator (A, '<=', B) ;
    end
end

