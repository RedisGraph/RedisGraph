function C = mldivide (A, B)
% C = A\B, matrix left division.
%
% If A is a scalar, then C = A.\B is computed; see 'help ldivide'.
% Otherwise, C is computed by first converting A and B to MATLAB sparse
% matrices, and then C=A\B is computed using the MATLAB backslash.
%
% The input matrices may be either GraphBLAS and/or MATLAB matrices, in
% any combination.  C is returned as a GraphBLAS matrix.
%
% See also GrB/mrdivide.

% FUTURE: add solvers over a group (GF(2) for example).

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (isscalar (A))
    C = rdivide (B, A) ;
else
    C = GrB (builtin ('mldivide', double (A), double (B))) ;
end

