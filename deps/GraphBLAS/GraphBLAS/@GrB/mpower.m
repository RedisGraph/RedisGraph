function C = mpower (A, B)
%A^B Matrix power.
% C = A^B computes the matrix power of A raised to the B. A must be a
% square matrix.  B must an integer >= 0.
%
% The inputs may be either GraphBLAS and/or MATLAB matrices/scalars, in
% any combination.  C is returned as a GraphBLAS matrix.
%
% See also GrB/power.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m, n] = size (A) ;

if (m ~= n)
    gb_error ('For C=A^B, A must be square') ;
end

if (~isscalar (B))
    gb_error ('For C=A^B, B must be a non-negative integer scalar') ;
end


b = gb_get_scalar (B) ;
if (isreal (b) && isfinite (b) && round (b) == b && b >= 0)
    if (b == 0)
        % C is identity, of the same type as A
        % FUTURE: ones (...) needs to be 'double' if A is complex.
        C = GrB.build (1:n, 1:n, ones (1, n, GrB.type (A)), n, n) ;
    else
        % C = A^b where b > 0 is an integer
        C = compute_mpower (A, b) ;
    end
else
    gb_error ('For C=A^B, B must be a non-negative integer scalar') ;
end

end

function C = compute_mpower (A, b)
% C = A^b where b > 0 is an integer
if (b == 1)
    C = A ;
else
    T = compute_mpower (A, floor (b/2)) ;
    C = T*T ;
    clear T ;
    if (mod (b, 2) == 1)
        C = C*A ;
    end
end
end

