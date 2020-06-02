function C = spones (G, type)
%SPONES return pattern of GraphBLAS matrix.
% C = spones (G) returns a matrix C with the same pattern as G, but with
% all entries set to 1.  The behavior of spones (G) for a GrB matrix
% differs from spones (A) for a MATLAB matrix A.  An explicit entry
% G(i,j) that has a value of zero is converted to the explicit entry
% C(i,j)=1.  Explicit zero entries never appear in a MATLAB sparse
% matrix.
%
% C = spones (G) returns C as the same type as G.
% C = spones (G,type) returns C in the requested type ('double',
% 'single', 'int8', ...).  For example, use C = spones (G, 'logical') to
% return the pattern of G as a sparse logical matrix.
%
% See also spfun, GrB.apply.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 1)
    C = GrB.apply ('1', G) ;
else
    if (~ischar (type))
        gb_error ('type must be a string') ;
    end
    C = GrB.apply (['1.' type], G) ;
end

