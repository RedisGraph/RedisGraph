function C = GB_spec_select (C, Mask, accum, opname, A, k, descriptor)
%GB_SPEC_SELECT a MATLAB mimic of GxB_select
%
% Usage:
% C = GB_spec_select (C, Mask, accum, opname, A, k, descriptor)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

%-------------------------------------------------------------------------------
% get inputs
%-------------------------------------------------------------------------------

if (nargout > 1 || nargin ~= 7)
    error ('usage: C = GB_spec_select (C, Mask, accum, opname, A, k, desc)');
end

C = GB_spec_matrix (C) ;
A = GB_spec_matrix (A) ;
Mask = GB_spec_getmask (Mask) ;
[C_replace Mask_comp Atrans ~] = GB_spec_descriptor (descriptor) ;

%-------------------------------------------------------------------------------
% do the work via a clean MATLAB interpretation of the entire GraphBLAS spec
%-------------------------------------------------------------------------------

% select the descriptor to A
if (Atrans)
    A.matrix = A.matrix' ;
    A.pattern = A.pattern' ;
end

[m n] = size (A.matrix) ;
T.matrix = zeros (m, n, A.class) ;

switch (opname)
    case 'tril'
        p = tril (A.pattern, k) ;
    case 'triu'
        p = triu (A.pattern, k) ;
    case 'diag'
        p = tril (triu (A.pattern, k), k) ;
    case 'offdiag'
        p = tril (A.pattern, k-1) | triu (A.pattern, k+1) ;
    case 'nonzero'
        p = A.pattern & (A.matrix ~= 0) ;
    otherwise
        error ('invalid op') ;
end

T.matrix (p) = A.matrix (p) ;
T.pattern = p ;

% C<Mask> = accum (C,T): select the accum, then Mask, and return the result
C = GB_spec_accum_mask (C, Mask, accum, T, C_replace, Mask_comp, 0) ;

