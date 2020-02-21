function w = GB_spec_Col_extract (w, mask, accum, A, I, j, descriptor)
%GB_SPEC_COL_EXTRACT a MATLAB mimic of GrB_Col_extract

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargout > 1 || nargin ~= 7)
    error ('usage: w = GB_spec_Col_extract (w, mask, accum, A, I, j, desc)');
end

% make sure j is a scalar
if (length (j) ~= 1)
    error ('j must be a scalar') ;
end

w = GB_spec_Matrix_extract (w, mask, accum, A, I, j, descriptor) ;



