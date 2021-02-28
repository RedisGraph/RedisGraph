function C = GB_spec_resize (A, nrows_new, ncols_new)
%GB_SPEC_RESIZE a MATLAB mimic of GxB_resize
%
% Usage:
% C = GB_spec_resize (A, nrows_new, ncols_new)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

%-------------------------------------------------------------------------------
% get inputs
%-------------------------------------------------------------------------------

if (nargout > 1 || nargin ~= 3)
    error ('usage: C = GB_spec_resize (A, nrows_new, ncols_new)') ;
end

C = GB_spec_matrix (A) ;
clas = C.class ;

%-------------------------------------------------------------------------------
% do the work via a clean MATLAB interpretation of the entire GraphBLAS spec
%-------------------------------------------------------------------------------

[nrows_old ncols_old] = size (C.matrix) ;

if (ncols_new < ncols_old)
    C.matrix  = C.matrix  (:, 1:ncols_new) ;
    C.pattern = C.pattern (:, 1:ncols_new) ;
elseif (ncols_new > ncols_old)
    C.matrix  = [C.matrix  (zeros (nrows_old, ncols_new - ncols_old, clas))] ;
    C.pattern = [C.pattern (false (nrows_old, ncols_new - ncols_old))] ;
end

if (nrows_new < nrows_old)
    C.matrix  = C.matrix  (1:nrows_new, :) ;
    C.pattern = C.pattern (1:nrows_new, :) ;
elseif (nrows_new > nrows_old)
    C.matrix  = [C.matrix  ; (zeros (nrows_new - nrows_old, ncols_new, clas))] ;
    C.pattern = [C.pattern ; (false (nrows_new - nrows_old, ncols_new))] ;
end

