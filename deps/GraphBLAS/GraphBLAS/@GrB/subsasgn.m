function C = subsasgn (C, S, A)
%SUBSASGN C(I,J) = A or C(I) = A; assign submatrix into a GraphBLAS matrix
% C(I,J) = A assigns A into the C(I,J) submatrix of the GraphBLAS matrix
% C.  A must be either a matrix of size length(I)-by-length(J), or a
% scalar.  Note that C(I,J) = 0 differs from C(I,J) = sparse (0).  The
% former places an explicit entry with value zero in all positions of
% C(I,J).  The latter deletes all entries in C(I,J).  With a MATLAB
% sparse matrix C, both statements delete all entries in C(I,J) since
% MATLAB never stores explicit zeros in its sparse matrices.
%
% With a single index, C(I) = A, both C and A must be vectors; linear
% indexing is not yet supported.  In this case A must either be a vector
% of length the same as I, or a scalar.
%
% If M is a logical matrix, C (M) = x is an assignment via logical
% indexing, where C and M have the same size, and x(:) is either a vector
% of length nnz (M), or a scalar.
%
% Note that C (M) = A (M), where the same logical matrix M is used on
% both the sides of the assignment, is identical to C = GrB.subassign (C,
% M, A).  If C and A (or M) are GraphBLAS matrices, C (M) = A (M) uses
% GraphBLAS via operator overloading.  The statement C (M) = A (M) takes
% about twice the time as C = GrB.subassign (C, M, A), so the latter is
% preferred for best performance.  However, both methods in GraphBLAS are
% many thousands of times faster than C (M) = A (M) using purely MATLAB
% sparse matrices C, M, and A, when the matrices are large.  So either
% method works fine, relatively speaking.
%
% If I or J are very large colon notation expressions, then C(I,J)=A is
% not possible, because MATLAB creates I and J as explicit lists first.
% See GrB.subassign instead.  See also the example with 'help GrB.extract'.
%
% See also subsref, GrB.assign, GrB.subassign.

% FUTURE: add linear indexing, and allow the matrix to grow in size.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (~isequal (S.type, '()'))
    error ('GrB:unsupported', 'index type %s not supported', S.type) ;
end

ndims = length (S.subs) ;
if (ndims == 1)
    if (isequal (GrB.type (S.subs {1}), 'logical'))
        % C (M) = A for logical assignment
        M = S.subs {1} ;
        % the 'all' syntax requires MATLAB R2019a
        % if (any (M, 'all'))
            if (isscalar (A))
                % C (M) = scalar
                C = GrB.subassign (C, M, A) ;
            else
                % C (M) = A where A is a vector
                if (isa (M, 'GrB'))
                    M = M.opaque ;
                end
                if (size (A, 2) ~= 1)
                    % make sure A is a column vector of size mnz-by-1
                    A = A (:) ;
                end
                if (isa (A, 'GrB'))
                    A = A.opaque ;
                end
                if (isa (C, 'GrB'))
                    C = C.opaque ;
                end
                C = GrB (gblogassign (C, M, A)) ;
            end
        % else
        %     % M is empty, so C does not change
        % end
    else
        % C (I) = A where C is a vector
        I = gb_get_index (S.subs (1)) ;
        C = GrB.subassign (C, I, A) ;
    end
elseif (ndims == 2)
    % C(I,J) = A where A is length(I)-by-length(J), or a scalar
    I = gb_get_index (S.subs (1)) ;
    J = gb_get_index (S.subs (2)) ;
    C = GrB.subassign (C, I, J, A) ;
else
    error ('GrB:unsupported', '%dD indexing not supported', ndims) ;
end

