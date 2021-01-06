function C = min (A, B, option)
%MIN Maximum elements of a matrix.
% C = min (A) is the smallest entry in the vector A.  If A is a matrix,
% C is a row vector with C(j) = min (A (:,j)).
%
% C = min (A,B) is an array of the element-wise minimum of two matrices
% A and B, which either have the same size, or one can be a scalar.
%
% C = min (A, [ ], 'all') is a scalar, with the smallest entry in A.
% C = min (A, [ ], 1) is a row vector with C(j) = min (A (:,j))
% C = min (A, [ ], 2) is a column vector with C(i) = min (A (i,:))
%
% The 2nd output of [C,I] = min (...) in the MATLAB built-in min
% is not yet supported.  The min (..., nanflag) option is
% not yet supported; only the 'omitnan' behavior is supported.
%
% Complex matrices are not supported.
%
% See also GrB/max.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

% FUTURE: min(A,B) for two matrices A and B is slower than it could be.
% See comments in gb_union_op.

if (isobject (A))
    A = A.opaque ;
end

type = gbtype (A) ;
if (contains (type, 'complex'))
    error ('complex matrices not yet supported') ;
elseif (isequal (type, 'logical'))
    op = '&.logical' ;
else
    op = 'min' ;
end

if (nargin == 1)
    % C = min (A)
    C = GrB (gb_min1 (op, A)) ;
elseif (nargin == 2)
    % C = min (A,B)
    if (isobject (B))
        B = B.opaque ;
    end
    C = GrB (gb_min2 (op, A, B)) ;
else
    % C = min (A, [ ], option)
    if (~isempty (B))
        error ('dimension argument not allowed with 2 input matrices') ;
    end
    C = GrB (gb_min3 (op, A, option)) ;
end

