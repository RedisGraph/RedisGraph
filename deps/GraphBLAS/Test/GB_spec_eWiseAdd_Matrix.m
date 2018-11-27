function C = GB_spec_eWiseAdd_Matrix (C, Mask, accum, add, A, B, descriptor)
%GB_SPEC_EWISEADD_MATRIX a MATLAB mimic of GrB_eWiseAdd_Matrix
%
% Usage:
% C = GB_spec_eWiseAdd_Matrix (C, Mask, accum, add, A, B, descriptor)
%
% Computes C<Mask> = accum(C,T), in GraphBLAS notation, where T =A+B, A'+B,
% A+B' or A'+B'.  The pattern of T is the union of A and B.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

%-------------------------------------------------------------------------------
% get inputs
%-------------------------------------------------------------------------------

if (nargout > 1 || nargin ~= 7)
    error ('usage: C = GB_spec_eWiseAdd_Matrix (C, Mask, accum, add, A, B, descriptor)') ;
end

C = GB_spec_matrix (C) ;
A = GB_spec_matrix (A) ;
B = GB_spec_matrix (B) ;
[add_op xyclass zclass] = GB_spec_operator (add, C.class) ;
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

% T = A+B, with typecasting
[nrows, ncols] = size (A.matrix) ;
T.matrix = zeros (nrows, ncols, zclass) ;

% apply the add to entries in the intersection of A and B
p = A.pattern & B.pattern ;
% first cast the entries into the class of the operator
% note that in the spec, all three domains z=op(x,y) can be different
% here they are assumed to all be the same
A1 = GB_mex_cast (A.matrix (p), xyclass) ;
B1 = GB_mex_cast (B.matrix (p), xyclass) ;
T.matrix (p) = GB_spec_op (add, A1, B1) ;

% cast entries in A but not in B, into the result T
p = A.pattern & ~B.pattern ;
T.matrix (p) = GB_mex_cast (A.matrix (p), zclass) ;

% cast entries in B but not in A, into the result T
p = B.pattern & ~A.pattern ;
T.matrix (p) = GB_mex_cast (B.matrix (p), zclass) ;

% the pattern of T is the union of both A and B
T.pattern = A.pattern | B.pattern ;

T.class = zclass ;

% C<Mask> = accum (C,T): apply the accum, then Mask, and return the result
C = GB_spec_accum_mask (C, Mask, accum, T, C_replace, Mask_comp, 0) ;


