function C = xor (A, B)
%XOR logical exclusive OR.
% C = xor (A,B) is the element-by-element logical OR of A and B.  One or
% both may be scalars.  Otherwise, A and B must have the same size.
% GraphBLAS and MATLAB matrices may be combined.
%
% See also GrB/and, GrB/or.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (isscalar (A))
    if (isscalar (B))
        % A and B are scalars
        C = GrB.emult (A, 'xor.logical', B) ;
    else
        % A is a scalar, B is a matrix
        if (gb_get_scalar (A) == 0)
            % A is false, so C is B typecasted to logical
            C = GrB (B, 'logical') ;
        else
            % A is true, so C is a full matrix the same size as B
            C = not (B) ;
        end
    end
else
    if (isscalar (B))
        % A is a matrix, B is a scalar
        if (gb_get_scalar (B) == 0)
            % B is false, so C is A typecasted to logical
            C = GrB (A, 'logical') ;
        else
            % B is true, so C is a full matrix the same size as A
            C = not (A) ;
        end
    else
        % both A and B are matrices.  C is the set union of A and B
        C = GrB.eadd (A, 'xor.logical', B) ;
    end
end

