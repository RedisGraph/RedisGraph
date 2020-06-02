function C = rdivide (A, B)
%TIMES C = A./B, sparse matrix element-wise division.
% C = A./B when B is a matrix results in a dense matrix C, with all
% entries present.  If A is a matrix and B is a scalar, then C has the
% pattern of A, except if B is zero and A is double, single, or complex.
% In that case, since 0/0 is NaN, C is a dense matrix.  If the types of A
% and B differ, C has the type of A, and B is typecasted into the type of
% A before computing C=A./B.
%
% The input matrices may be either GraphBLAS and/or MATLAB matrices, in
% any combination.  C is returned as a GraphBLAS matrix.
%
% See also rdivide, GrB.emult, GrB.eadd.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (isscalar (A))
    if (isscalar (B))
        % both A and B are scalars
    else
        % A is a scalar, B is a matrix.  A is expanded to full
        [m, n] = size (B) ;
        % A (1:m,1:n) = A and cast to the type of B
        A = GrB.subassign (GrB (m, n, GrB.type (B)), A) ;
    end
else
    if (isscalar (B))
        % A is a matrix, B is a scalar
        if (gb_get_scalar (B) == 0 && isfloat (A))
            % 0/0 is Nan, and thus must be computed computed if A is
            % floating-point.  The result is a dense matrix.
            [m, n] = size (A) ;
            % B (1:m,1:n) = B and cast to the type of A
            B = GrB.subassign (GrB (m, n, GrB.type (A)), B) ;
        else
            % The scalar B is nonzero so just compute A/B in the pattern
            % of A.  The result is sparse (the pattern of A).
            B = GrB.expand (B, A) ;
        end
    else
        % both A and B are matrices.  The result is a dense matrix.
        if (~GrB.isfull (A))
            A = full (A) ;
        end
        if (~GrB.isfull (B))
            B = full (B) ;
        end
    end
end

C = GrB.emult (A, '/', B) ;

