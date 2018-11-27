function C = GB_spec_transpose (C, Mask, accum, A, descriptor)
%GB_SPEC_TRANSPOSE a MATLAB mimic of GrB_transpose
%
% Usage:
% C = GB_spec_transpose (C, Mask, accum, A, descriptor)
%
% Computes C<Mask> = accum(C,A') or accum(C,A) depending on the descriptor.
% This function and all GB_spec_* functions are not mean to be fast.  They
% are meant to represent the GraphBLAS spec, for testing and illustration.
%
% The input matrices A, Mask, and C can be plain MATLAB matrices, sparse or
% dense, where the class of the matrix is the same as the GraphBLAS type.
% MATLAB sparse matrices can only handle 'logical' or 'double' classes,
% however.  To model this, the input matrices can also be structs.  A.matrix is
% the matrix itself, and A.class is a string that the matrix is supposed to
% represent ('logical', 'int8', ... 'single', 'double').
%
% accum can be a string with the name a valid binary operator (see
% GB_spec_operator.m), or it can be a struct where accum.name is the name of
% the operator and accum.class is the class.  If accum is empty, no accumulator
% is used and C on input is ignored.
%
% Mask is a sparse or dense matrix, or an empty matrix ([ ]).  See
% GB_spec_mask.m for a description.
%
% descriptor is a optional struct.  Defaults are used if empty or not present.
%       descriptor.outp = 'replace' (clear C first) or 'default' (use C as-is)
%       descriptor.inp0 = 'tran' (do C=A) or 'default' (do C=A')
%       descriptor.mask = 'scmp' (use ~Mask) or 'default' (use Mask)
%
% GB_spec_transpose implements the entire specification of GrB_transpose, with
% a few exceptions.
%
% (1) Internally it only works on dense matrices, since MATLAB supports logical
% and double sparse matrices only.  As a result, the MATLAB GB_spec_* mimic
% functions will be slow on large problems.
%
% (2) MATLAB does not allow explicit zeros in its sparse matrices. As a result,
% the structural pattern of an input matrix A, sparse or dense, is assumed to
% be spones(A).
%
% (3) Operations cannot be done purely in MATLAB because of the differences in
% typecasting rules for integers.
%
% (4) Finally, this method assumes that the domains of x, y, and z for the
% operator z=op(x,y) are all the same: opclass.  This is true for all built-in
% operators, but in GraphBLAS users can define their own operators with x, y
% and z being all different domains.  That feature is not modeled by this
% MATLAB function.
%
% Returns a struct with a dense matrix C.matrix with class C.class.
%
% Use an empty value ([ ] or '') to obtain the default value for optional
% parameters.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

%-------------------------------------------------------------------------------
% get inputs
%-------------------------------------------------------------------------------

if (nargout > 1 || nargin ~= 5)
    error ('usage: C = GB_spec_transpose (C, Mask, accum, A, descriptor)') ;
end

C = GB_spec_matrix (C) ;
A = GB_spec_matrix (A) ;
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

% do the work

% Yes, this is be a double-transpose if Atrans is true, which of course should
% not be done in practice.  This just mimics the spec line-for-line.
T.matrix = A.matrix' ;
T.pattern = A.pattern' ;
T.class = A.class ;

% C<Mask> = accum (C,T): apply the accum, then Mask, and return the result
C = GB_spec_accum_mask (C, Mask, accum, T, C_replace, Mask_comp, 0) ;

