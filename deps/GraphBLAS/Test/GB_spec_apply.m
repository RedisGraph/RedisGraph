function C = GB_spec_apply (C, Mask, accum, op, A, descriptor)
%GB_SPEC_APPLY a MATLAB mimic of GrB_apply
%
% Usage:
% C = GB_spec_apply (C, Mask, accum, op, A, descriptor)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

%-------------------------------------------------------------------------------
% get inputs
%-------------------------------------------------------------------------------

if (nargout > 1 || nargin ~= 6)
    error ('usage: C = GB_spec_apply (C, Mask, accum, op, A, descriptor)') ;
end

C = GB_spec_matrix (C) ;
A = GB_spec_matrix (A) ;
[opname xyclass zclass] = GB_spec_operator (op, C.class) ;
Mask = GB_spec_getmask (Mask) ;
[C_replace Mask_comp Atrans ~] = GB_spec_descriptor (descriptor) ;

%-------------------------------------------------------------------------------
% do the work via a clean MATLAB interpretation of the entire GraphBLAS spec
%-------------------------------------------------------------------------------

% apply the descriptor to A
if (Atrans)
    A.matrix = A.matrix' ;
    A.pattern = A.pattern' ;
end

% T = op(A)
[m n] = size (A.matrix) ;
T.matrix = zeros (m, n, zclass) ;
T.pattern = A.pattern ;
T.class = zclass ;

A_matrix = GB_mex_cast (A.matrix, xyclass) ;

p = T.pattern ;
T.matrix (p) = GB_spec_op (op, A.matrix (p)) ;

% C<Mask> = accum (C,T): apply the accum, then Mask, and return the result
C = GB_spec_accum_mask (C, Mask, accum, T, C_replace, Mask_comp, 0) ;

