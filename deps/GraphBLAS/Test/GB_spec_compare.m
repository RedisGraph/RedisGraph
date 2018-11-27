function ok = GB_spec_compare (C_spec, C_mex, identity, tol)
%GB_SPEC_COMPARE compare MATLAB mimic result with GraphBLAS result
% ok = GB_spec_compare (C_spec, C_mex, identity)
%
% compares two structs C_spec and C_mex.  The C_spec struct contains a dense
% matrix and is the output of a MATLAB mimic, C_spec = GB_spec_* (...) for
% some GraphBLAS method.  C_mex = GrG_mex_* (...) is the output of the
% corresponding MATLAB interface to the true GraphBLAS method, in C.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% get the semiring identity
if (nargin < 3)
    identity = 0 ;
end

if (nargin < 4)
    if (isfloat (identity))
        tol = 64*eps (class (identity)) ;
    else
        tol = 0 ;
    end
end

% Convert C_mex from a sparse matrix into a dense matrix.  It will have
% explicit identity values where entries were not in the pattern of the sparse
% C_mex.matrix.  Entries outside the pattern are "don't care" values.  They may
% differ between C_spec and C_mex, because the latter works on dense matrices
% and thus must compute the entries.  With typecasting, an identity value may
% get modified.  C_mex, on the other hand, is computed in a GraphBLAS sparse
% matrix, and never appears.  It is only converted here to a dense matrix, with
% the implicit entries being replaced with identity.  C_spec is also converted,
% using the same identity value.
C1 = GB_spec_matrix (C_spec, identity) ;
C2 = GB_spec_matrix (C_mex, identity) ;

try
    % ok_matrix = isequalwithequalnans (C1.matrix, C2.matrix) ;
    ok_matrix = isequal_roundoff (C1.matrix, C2.matrix, tol) ;
catch
    ok_matrix = false ;
end

try
    ok_pattern = isequal (C1.pattern, C2.pattern) ;
catch
    ok_pattern = false ;
end

try
    ok_class = isequal (C1.class, C2.class) ;
catch
    ok_class = false ;
end

%{
if (~ok_class)
    fprintf ('class is wrong:\n') ;
    % C1.class
    % C2.class
end

if (~ok_matrix)
    fprintf ('matrix is wrong:\n') ;
    identity
    % C1.matrix
    % C2.matrix
end

if (~ok_pattern)
    fprintf ('pattern is wrong:\n') ;
    % C1.pattern
    % C2.pattern
end
%}

if (~ok_class || ~ok_pattern || ~ok_matrix)
    % C_spec
    % % C_mex
    % C1
    % C2
    fprintf ('matrix: %d pattern: %d class %d\n', ...
        ok_matrix, ok_pattern, ok_class) ;
end

% with no output, just assert that ok is true
if (nargout == 0)
    assert (ok_matrix && ok_pattern && ok_class) ;
end


