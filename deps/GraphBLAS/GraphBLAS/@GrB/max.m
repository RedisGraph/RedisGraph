function C = max (A, B, option)
%MAX Maximum elements of a matrix.
% C = max (A) is the largest entry in the vector A.  If A is a matrix,
% C is a row vector with C(j) = max (A (:,j)).
%
% C = max (A,B) is an array of the element-wise maximum of two matrices
% A and B, which either have the same size, or one can be a scalar.
%
% C = max (A, [ ], 'all') is a scalar, with the largest entry in A.
% C = max (A, [ ], 1) is a row vector with C(j) = max (A (:,j))
% C = max (A, [ ], 2) is a column vector with C(i) = max (A (i,:))
%
% The 2nd output of [C,I] = max (...) in the MATLAB built-in max
% is not yet supported.  The max (..., nanflag) option is
% not yet supported; only the 'omitnan' behavior is supported.
%
% Complex matrices are not supported.
%
% See also GrB/min.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

% FUTURE: max(A,B) for two matrices A and B is slower than it could be.
% See comments in gb_union_op.

if (isobject (A))
    A = A.opaque ;
end

type = gbtype (A) ;
if (contains (type, 'complex'))
    error ('complex matrices not yet supported') ;
elseif (isequal (type, 'logical'))
    op = '|.logical' ;
else
    op = 'max' ;
end

if (nargin == 1)
    % C = max (A)
    C = GrB (gb_max1 (op, A)) ;
elseif (nargin == 2)
    % C = max (A,B)
    if (isobject (B))
        B = B.opaque ;
    end
    C = GrB (gb_max2 (op, A, B)) ;
else
    % C = max (A, [ ], option)
    if (~isempty (B))
        error ('dimension argument not allowed with 2 input matrices') ;
    end
    C = GrB (gb_max3 (op, A, option)) ;
end

