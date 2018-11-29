function w = GB_spec_reduce_to_vector (w, mask, accum, reduce, A, descriptor)
%GB_SPEC_REDUCE_TO_VECTOR a MATLAB mimic of GrB_reduce (to vector)
%
% Usage:
% w = GB_spec_reduce_to_vector (w, mask, accum, reduce, A, desc)
%
% Reduces a matrix to a vector

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

%-------------------------------------------------------------------------------
% get inputs
%-------------------------------------------------------------------------------

if (nargout > 1 || nargin ~= 6)
    error ...
    ('usage: c = GB_spec_reduce_to_vector (w, mask, accum, reduce, A, desc)') ;
end

% get the class of A
if (isstruct (A))
    aclass = A.class ;
else
    aclass = class (A) ;
end

% get the reduce operator. default class is the class of A
if (isempty (reduce))
    reduce = 'plus'
end
[reduce_op reduce_class] = GB_spec_operator (reduce, aclass) ;

% get the identity
identity = GB_spec_identity (reduce_op, reduce_class) ;

% get the input matrix
A = GB_spec_matrix (A, identity) ;

% get the input vector
w = GB_spec_matrix (w, identity) ;

% get the mask
mask = GB_spec_getmask (mask) ;

[C_replace Mask_comp Atrans ~] = GB_spec_descriptor (descriptor) ;

%-------------------------------------------------------------------------------
% do the work via a clean MATLAB interpretation of the entire GraphBLAS spec
%-------------------------------------------------------------------------------

% apply the descriptor to A
if (Atrans)
    A.matrix = A.matrix' ;
    A.pattern = A.pattern' ;
end

tclass = reduce_class ;
[m n] = size (A.matrix) ;
T.matrix = zeros (m, 1, tclass) ;
T.pattern = zeros (m, 1, 'logical') ;
T.matrix (:,:) = identity ;
T.class = tclass ;

% cast A to the type of the reduce operator, but only entries in the pattern

% Note that GraphBLAS does not need the identity value to do this step.
% Nor is the identity value specified in the GraphBLAS spec.
% It is needed here since T is a dense column, not sparse, and the
% implicit values need to be inserted properly.

for i = 1:m
    t = identity ;
    for j = 1:n
        if (A.pattern (i,j))
            aij = GB_mex_cast (A.matrix (i,j), reduce_class) ;
            t = GB_spec_op (reduce, t, aij) ;
            T.pattern (i) = true ;
        end
    end
    T.matrix (i) = t ;
end

%-------------------------------------------------------------------------------
% apply the mask and accum
%-------------------------------------------------------------------------------

% w<mask> = accum (C,T): apply the accum, then mask, and return the result
w = GB_spec_accum_mask (w, mask, accum, T, C_replace, Mask_comp, identity) ;

