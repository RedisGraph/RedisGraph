function Cout = select (varargin)
%GRB.SELECT: select entries from a GraphBLAS sparse matrix.
%
% Usage:
%
%   Cout = GrB.select (selectop, A)
%   Cout = GrB.select (selectop, A, b)
%   Cout = GrB.select (selectop, A, b, desc)
%
%   Cout = GrB.select (Cin, accum, selectop, A)
%   Cout = GrB.select (Cin, accum, selectop, A, b)
%   Cout = GrB.select (Cin, accum, selectop, A, b, desc)
%
%   Cout = GrB.select (Cin, M, selectop, A)
%   Cout = GrB.select (Cin, M, selectop, A, b)
%   Cout = GrB.select (Cin, M, selectop, A, b, desc)
%
%   Cout = GrB.select (Cin, M, accum, selectop, A)
%   Cout = GrB.select (Cin, M, accum, selectop, A, b)
%   Cout = GrB.select (Cin, M, accum, selectop, A, b, desc)
%
% GrB.select selects a subset of entries from the matrix A, based on
% their value or position.  For example, L = GrB.select ('tril', A, 0)
% returns the lower triangular part of the GraphBLAS or MATLAB matrix A,
% just like L = tril (A) for a MATLAB matrix A.  The select operators can
% also depend on the values of the entries.  The b parameter is an
% input scalar, used in many of the select operators.  For example,
% L = GrB.select ('tril', A, -1) is the same as L = tril (A, -1), which
% returns the strictly lower triangular part of A.  The b scalar is
% required for 'tril', 'triu', 'diag', 'offdiag' and the 2-input
% operators.  It must not appear when using the '*0' operators.
%
% The selectop is a string defining the operator:
%
%   operator        MATLAB equivalent           alternative strings
%   --------        -----------------           -------------------
%   'tril'          C = tril (A,b)
%   'triu'          C = triu (A,b)
%   'diag'          C = diag (A,b), see note
%   'offdiag'       C = entries not in diag(A,b)
%   'nonzero'       C = A (A ~= 0)              '~=0'
%   'zero'          C = A (A == 0)              '==0'
%   'positive'      C = A (A >  0)              '>0'
%   'nonnegative'   C = A (A >= 0)              '>=0'
%   'negative'      C = A (A <  0)              '<0'
%   'nonpositive'   C = A (A <= 0)              '<=0'
%   '~='            C = A (A ~= b)
%   '=='            C = A (A == b)
%   '>'             C = A (A >  b)
%   '>='            C = A (A >= b)
%   '<'             C = A (A <  b)
%   '<='            C = A (A <= b)
%
% Note that C = GrB.select ('diag',A,b) does not returns a vector,
% but a diagonal matrix.
%
% Many of the operations have equivalent synonyms, as listed above.
%
% Cin is an optional input matrix.  If Cin is not present or is an empty
% matrix (Cin = [ ]) then it is implicitly a matrix with no entries, of
% the right size (which depends on A, and the descriptor).  Its type is
% the output type of the accum operator, if it is present; otherwise, its
% type is the type of the matrix A.
%
% M is the optional mask matrix.  If not present, or if empty, then no
% mask is used.  If present, M must have the same size as C.
%
% If accum is not present, then the operation becomes C<...> =
% select(...).  Otherwise, accum (C, select(...)) is computed.  The accum
% operator acts like a sparse matrix addition (see GrB.eadd).
%
% The selectop is a required string defining the select operator to use.
% All operators operate on all types (the select operators do not do any
% typecasting of its inputs).
%
% A is the input matrix.  It is transposed on input if desc.in0 =
% 'transpose'.
%
% The descriptor desc is optional.  If not present, all default settings
% are used.  Fields not present are treated as their default values.  See
% 'help GrB.descriptorinfo' for more details.
%
% All input matrices may be either GraphBLAS and/or MATLAB matrices, in
% any combination.  Cout is returned as a GraphBLAS matrix, by default;
% see 'help GrB/descriptorinfo' for more options.
%
% See also tril, triu, diag.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[args, is_gb] = gb_get_args (varargin {:}) ;
if (is_gb)
    Cout = GrB (gbselect (args {:})) ;
else
    Cout = gbselect (args {:}) ;
end

