function [x no_value] = GB_spec_Vector_extractElement (A, i, xclass)
%
%GB_SPEC_VECTOR_EXTRACTELEMENT a MATLAB mimic of GrB_Matrix_extractElement

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (size (A,2) ~= 1)
    error ('invalid vector') ;
end

[x no_value] = GB_spec_Matrix_extractElement (A, i, 0, xclass) ;


