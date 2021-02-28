function [Z simple] = GB_spec_accum (accum, C, T, identity)
%GB_SPEC_ACCUM MATLAB mimic of the Z=accum(C,T) operation in GraphBLAS
%
% Z = GB_spec_accum (accum, C, T, identity)
%
% Apply accum binary operator to the input C and the intermediate result T.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% get the operator; default is class(C) if class is not present
[opname opclass] = GB_spec_operator (accum, C.class) ;

if (nargin < 4)
    identity = 0 ;
end

% simple is true if the simpler accum_mask.m script will work without
% any typecast, and with a zero identity
simple = (identity == 0) ;

% initialize the matrix Z, same size and class as C
[nrows ncols] = size (C.matrix) ;
Z.matrix  = zeros (nrows, ncols, C.class) ;
% Z.matrix (:,:) = GB_mex_cast (identity, C.class) ;
Z.pattern = false (nrows, ncols) ;
Z.class = C.class ;

if (isempty (opname))

    % Z = T, casting into the class of C
    p = T.pattern ;
    Z.matrix (p) = GB_mex_cast (T.matrix (p), C.class) ;
    Z.pattern = T.pattern ;

    simple = simple && isfield (T, 'class') && isequal (T.class, C.class) ;

else

    % Z = accum (C,T)

    % apply the operator to entries in the intersection of C and T
    p = T.pattern & C.pattern ;
    % first cast the entries into the class of the operator
    % note that in the spec, all three domains z=op(x,y) can be different
    % here they are assumed to all be the same
    c = GB_mex_cast (C.matrix (p), opclass) ;
    t = GB_mex_cast (T.matrix (p), opclass) ;
    z = GB_spec_op (accum, c, t) ;
    % cast the result z from opclass into the class of C
    Z.matrix (p) = GB_mex_cast (z, C.class) ;

    % copy entries in C but not in T, into the result Z, no typecasting needed
    p = C.pattern & ~T.pattern ;
    Z.matrix (p) = C.matrix (p) ;

    % cast entries in T but not in C, into the result Z
    p = T.pattern & ~C.pattern ;
    Z.matrix (p) = GB_mex_cast (T.matrix (p), C.class) ;

    % the pattern of Z is the union of both T and C
    Z.pattern = C.pattern | T.pattern ;

    simple = simple && ( ...
        isequal (C.class, opclass) && ...
        isfield (T, 'class') && ...
        isequal (T.class, opclass) && ...
        isequal (class (z), C.class) && ...
        isequal (T.class, C.class)) ;
end

