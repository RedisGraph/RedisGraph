function C = subsasgn (C, S, A)
%SUBSASGN C(I,J) = A or C(I) = A; assign submatrix.
% C(I,J) = A assigns A into the C(I,J) submatrix of the GraphBLAS matrix
% C.  A must be either a matrix of size length(I)-by-length(J), or a
% scalar.  Note that C(I,J) = 0 differs from C(I,J) = sparse (0).  The
% former places an explicit entry with value zero in all positions of
% C(I,J).  The latter deletes all entries in C(I,J).  With a built-in
% sparse matrix C, both statements delete all entries in C(I,J) since
% Built-in sparse matrices never include explicit zeros.
%
% With a single index, C(I) = A, both C and A must be vectors; linear
% indexing is not yet supported.  In this case A must either be a vector
% of length the same as I, or a scalar.
%
% If M is a logical matrix, C (M) = x is an assignment via logical
% indexing, where C and M have the same size, and x(:) is either a vector
% of length nnz (M), or a scalar.
%
% C (M) = A (M), where the logical matrix M is used on both the sides of
% the assignment, is the same as C = GrB.subassign (C, M, A).  If C and A
% (or M) are GraphBLAS matrices, C (M) = A (M) uses GraphBLAS via operator
% overloading.  The statement C (M) = A (M) takes about twice the time as
% C = GrB.subassign (C, M, A), so the latter is preferred for best
% performance.  However, both methods in GraphBLAS are many thousands of
% times faster than C (M) = A (M) using purely built-in sparse matrices C,
% M, and A, when the matrices are large.
%
% If I or J are very large colon notation expressions, then C(I,J)=A is
% not possible, because I and J are created as explicit lists first,
% before passing them to GraphBLAS.  See GrB.subassign instead.  See also
% the example with 'help GrB.extract'.
%
% Unlike the built-in C(I,J)=A, the GraphBLAS assignment does not change
% the size of C.
%
% See also GrB/subsref, GrB/subsindex, GrB.assign, GrB.subassign.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

% FUTURE: add linear indexing, and allow the matrix to grow/shrink in size.

if (~isequal (S.type, '()'))
    error ('index type %s not supported', S.type) ;
end

if (isobject (C))
    C = C.opaque ;
end

if (isobject (A))
    A = A.opaque ;
end

ndims = length (S.subs) ;

if (ndims == 1)

    % C (M) = A if M is logical, or C (I) = A otherwise
    S = S.subs {1} ;
    if (isobject (S))
        S = S.opaque ;
    end
    if (isequal (gbtype (S), 'logical'))
        % C (M) = A for logical assignment
        [am, an] = gbsize (A) ;
        if (am == 1 && an == 1)
            % C (M) = scalar
            C = GrB (gbsubassign (C, S, A)) ;
        else
            % C (M) = A where A is a vector
            C = GrB (gblogassign (C, S, A)) ;
        end
    else
        % C (I) = A
        [cm, cn] = gbsize (C) ;
        if (cm == 1 || cn == 1)
            % C (I) = A for a vector or scalar C
            C = GrB (gbsubassign (C, gb_index (S), A)) ;
        else
            % C (I) = A for a matrix C
            error ('Linear indexing not yet supported') ;
        end
    end

elseif (ndims == 2)

    % C(I,J) = A where A is length(I)-by-length(J), or a scalar
    C = GrB (gbsubassign (C, gb_index (S.subs {1}), gb_index (S.subs {2}), A)) ;

else

    error ('%dD indexing not yet supported', ndims) ;

end

