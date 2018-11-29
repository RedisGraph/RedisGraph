function C = GB_spec_mxm (C, Mask, accum, semiring, A, B, descriptor)
%GB_SPEC_MXM a MATLAB mimic of GrB_mxm
%
% Usage:
% C = GB_spec_mxm (C, Mask, accum, semiring, A, B, descriptor)
%
% Computes C<Mask> = accum(C,T), in GraphBLAS notation, where T =A*B, A'*B,
% A*B' or A'*B'.  The matrix C is returned as a struct with C.matrix being the
% values of the matrix, C.pattern the 'nonzero' pattern, and C.class the class
% of the matrix.  A and B can be plain matrices on input, or they can be
% structs like C.  See GB_spec_matrix.m for more details.  See
% GB_spec_transpose.m for a description of the input parameters, except for
% semiring; see GB_spec_semiring for a description of the semiring.
%
% The semiring defines the multiply and add operators and the additive
% identity.  The matrix multiplication T = A*B is then defined as:
%
%   [m s] = size (A) ;
%   [s n] = size (B) ;
%   Tclass = 'logical' if semiring.multiply is 'eq', 'ne', 'gt', 'lt', 'ge',
%           or 'le', or semiring.class otherwise
%   T = identity (m,n,Tclass) ; where identity is defined by semiring.add
%   for j = 1:n
%       for i = 1:m
%           for k = 1:s
%               if (A (i,k) and B (k,j) are 'nonzero'
%                   aik = cast (A(i,k), semiring.class)
%                   bkj = cast (B(k,j), semiring.class)
%                   T (i,j) = add (T (i,j), multiply (aik, bkj))
%               end
%           end
%       end
%   end
%
% Where A(i,j) 'nonzero' means that the entry is in the data structure for the
% sparse matrix A.  If it is not there, it is implied to be equal to the
% addititive identity.  The identity value does not appear in the sparse
% GraphBLAS matrix; it is the value of entries not present in the data
% structure.  The MATLAB GB_spec_*.m functions operate on dense matrices,
% however, so these entries must be explicitly set to the additive identity.
%
% This gives a matrix T which is then accumulated into the result via
% C<Mask> = accum (C,T).  See GrB_accum_mask for a description of this
% last step.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

%-------------------------------------------------------------------------------
% get inputs
%-------------------------------------------------------------------------------

if (nargout > 1 || nargin ~= 7)
    error ('usage: C = GB_spec_mxm (C, Mask, accum, semiring, A, B, descriptor)') ;
end

% Convert inputs to dense matrices with explicit patterns and classes,
% and with where X(~X.pattern)==identity for all matrices A, B, and C.
[multiply add identity tclass] = GB_spec_semiring (semiring) ;
C = GB_spec_matrix (C, identity) ;
A = GB_spec_matrix (A, identity) ;
B = GB_spec_matrix (B, identity) ;
% Mask is a dense logical matrix, not a struct
Mask = GB_spec_getmask (Mask) ;
[C_replace Mask_comp Atrans Btrans] = GB_spec_descriptor (descriptor) ;

%-------------------------------------------------------------------------------
% do the work via a clean MATLAB interpretation of the entire GraphBLAS spec
%-------------------------------------------------------------------------------

% apply the descriptor to A
if (Atrans)
    A.matrix = A.matrix' ;
    A.pattern = A.pattern' ;
end

% apply the descriptor to B
if (Btrans)
    B.matrix = B.matrix' ;
    B.pattern = B.pattern' ;
end

% T = A*B
[m s] = size (A.matrix) ;
[s n] = size (B.matrix) ;
T.matrix = zeros (m, n, tclass) ;
T.pattern = zeros (m, n, 'logical') ;
T.matrix (:,:) = identity ;
T.class = tclass ;

A_matrix = GB_mex_cast (A.matrix, semiring.class) ;
B_matrix = GB_mex_cast (B.matrix, semiring.class) ;

for j = 1:n
    for i = 1:m
        for k = 1:s
            % T (i,j) += A (i,k) * B (k,j), using the semiring
            if (A.pattern (i,k) && B.pattern (k,j))
                z = GB_spec_op (multiply, A_matrix (i,k), B_matrix (k,j)) ;
                T.matrix (i,j) = GB_spec_op (add, T.matrix (i,j),  z)  ;
                T.pattern (i,j) = true ;
            end
        end
    end
end

% C<Mask> = accum (C,T): apply the accum, then Mask, and return the result
C = GB_spec_accum_mask (C, Mask, accum, T, C_replace, Mask_comp, identity) ;
