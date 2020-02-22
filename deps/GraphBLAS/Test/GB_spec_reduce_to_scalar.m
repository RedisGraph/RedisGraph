function c = GB_spec_reduce_to_scalar (cin, accum, reduce, A)
%GB_SPEC_REDUCE_TO_SCALAR a MATLAB mimic of GrB_reduce (to scalar)
%
% Usage:
% c = GB_spec_reduce_to_scalar (cin, accum, reduce, A)
%
% Reduces a matrix or vector to a scalar
%
% cin is a dense scalar

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

%-------------------------------------------------------------------------------
% get inputs
%-------------------------------------------------------------------------------

if (nargout > 1 || nargin ~= 4)
    error ('usage: c = GB_spec_Matrix_reduce (cin, accum, reduce, A)') ;
end

if (isstruct (cin) || issparse (cin) || ~isscalar (cin))
    error ('cin must be a dense scalar') ;
end

cin_class = class (cin) ;

% get the class of A
if (isstruct (A))
    aclass = A.class ;
else
    aclass = class (A) ;
end

% get the reduce operator. default class is the class of A
[reduce_op reduce_class] = GB_spec_operator (reduce, aclass) ;

% get the identity
identity = GB_spec_identity (reduce_op, reduce_class) ;
if (isempty (identity))
    identity = 0 ;
end

% get the input matrix
A = GB_spec_matrix (A, identity) ;

% get the accumulator and its 2 classes.
% accum_class is the class of x and y, and zclass is the class of z,
% for z = accum(x,y)
[accum_op  accum_class zclass ] = GB_spec_operator (accum, cin_class) ;

%-------------------------------------------------------------------------------
% do the work via a clean MATLAB interpretation of the entire GraphBLAS spec
%-------------------------------------------------------------------------------

% cast A to the type of the reduce operator, but only entries in the pattern

t = identity ;
if (nnz (A.pattern) > 0)
    X = GB_mex_cast (A.matrix (A.pattern), reduce_class) ;
    for k = 1:length (X)
        t = GB_spec_op (reduce, t, X (k)) ;
    end
end

% apply the accumulator.  no mask. t is a dense matrix so this is
% different than GB_spec_accum.
if (isempty (accum_op))
    c = GB_mex_cast (t, cin_class) ;
else
    % c = cin "+" t, but typecast all the arguments
    t   = GB_mex_cast (t,   accum_class) ;     % cast t to accum x class
    cin = GB_mex_cast (cin, accum_class) ;     % cast cin to accum y class
    z = GB_spec_op (accum, cin, t) ;           % z = accum (cin,t)
    c = GB_mex_cast (z, cin_class) ;           % cast z to class of cin
end

