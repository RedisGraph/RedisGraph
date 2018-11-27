function C = GB_spec_Matrix_extract (C, Mask, accum, A, I, J, descriptor)
%GB_SPEC_MATRIX_EXTRACT a MATLAB mimic of GrB_Matrix_extract
%
% Usage:
% C = GB_spec_Matrix_extract (C, Mask, accum, A, I, J, descriptor)
%
% MATLAB mimic of C<Mask> = accum (A (I,J))

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

%-------------------------------------------------------------------------------
% get inputs
%-------------------------------------------------------------------------------

if (nargout > 1 || nargin ~= 7)
    error ('usage: C = GB_spec_Matrix_extract (C, Mask, accum, A, I, J, descriptor)') ;
end

C = GB_spec_matrix (C) ;
A = GB_spec_matrix (A) ;
Mask = GB_spec_getmask (Mask) ;
[C_replace, Mask_comp, Atrans, ~] = GB_spec_descriptor (descriptor) ;

%-------------------------------------------------------------------------------
% do the work via a clean MATLAB interpretation of the entire GraphBLAS spec
%-------------------------------------------------------------------------------

% apply the descriptor to A
if (Atrans)
    A.matrix = A.matrix' ;
    A.pattern = A.pattern' ;
end

% expand I and J if empty
if (ischar (I) & isempty (I))
    % I = '' is treated as the empty list
    I = [ ] ;
elseif (isempty (I) || isequal (I, ':'))
    % I = [ ] is treated as ":"
    nrows = size (A.matrix, 1) ;
    I = 1:nrows ;
end
if (ischar (J) & isempty (J))
    % J = '' is treated as the empty list
    J = [ ] ;
elseif (isempty (J) || isequal (J, ':'))
    % J = [ ] is treated as the ":"
    ncols = size (A.matrix, 2) ;
    J = 1:ncols ;
end

T.matrix = A.matrix (I,J) ;
T.pattern = A.pattern (I,J) ;
T.class = A.class ;

% C<Mask> = accum (C,T): apply the accum, then Mask, and return the result
C = GB_spec_accum_mask (C, Mask, accum, T, C_replace, Mask_comp, 0) ;

