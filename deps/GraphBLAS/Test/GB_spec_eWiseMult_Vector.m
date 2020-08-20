function w = GB_spec_eWiseMult_Vector (w, mask, accum, mult, u, v, descriptor)
%GB_SPEC_EWISEMULT_VECTOR a MATLAB mimic of GrB_eWiseMult_Vector
%
% Usage:
% w = GB_spec_eWiseMult_Vector (w, mask, accum, mult, u, v, descriptor)
%
% Computes w<mask> = accum(w,t), in GraphBLAS notation, where t =u.*v,
% The pattern of t is the union of u and v.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

%-------------------------------------------------------------------------------
% get inputs
%-------------------------------------------------------------------------------

if (nargout > 1 || nargin ~= 7)
    error ('usage: C = GB_spec_eWiseMult_Vector (w, mask, accum, mult, u, v, descriptor)') ;
end

% make sure u is a column vector
if (isstruct (u))
    n = size (u.matrix, 2) ;
else
    n = size (u, 2);
end
if (n ~= 1)
    error ('u must be a vector') ;
end

% GraphBLAS does not allow u or v to be transposed via the descriptor
if (isfield (descriptor, 'inp0'))
    descriptor = rmfield (descriptor, 'inp0') ;
end
if (isfield (descriptor, 'inp1'))
    descriptor = rmfield (descriptor, 'inp1') ;
end

w = GB_spec_eWiseMult_Matrix (w, mask, accum, mult, u, v, descriptor) ;


