function C = pow2 (A, B)
%POW2 base-2 power and scale floating-point number.
% C = pow2 (A) is C(i,j) = 2.^A(i,j) for each entry in A.
% Since 2^0 is nonzero, C is a full matrix.
%
% C = pow2 (F,E) is C = F .* (2 .^ fix (E)).  C is sparse, with
% the same pattern as F+E.  Any imaginary parts of F and E are ignored.
%
% See also GrB/log2, GrB/power, GrB/exp.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

% FUTURE: pow2(A,B) for two matrices A and B is slower than it could be.
% See comments in gb_union_op.

if (isobject (A))
    A = A.opaque ;
end
atype = gbtype (A) ;

if (nargin == 1)
    % C = 2.^A
    if (~gb_isfloat (atype))
        atype = 'double' ;
    end
    C = GrB (gbapply ('pow2', gbfull (A, atype))) ;
else
    % C = A.*(2.^B)
    if (isobject (B))
        B = B.opaque ;
    end
    type = gboptype (atype, gbtype (B)) ;
    if (contains (type, 'single'))
        type = 'single' ;
    else
        type = 'double' ;
    end
    C = GrB (gb_union_op (['pow2.' type], A, B)) ;
end

