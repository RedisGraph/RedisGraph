function Cout = apply (varargin)
%GRB.APPLY apply a unary operator to a matrix.
%
% Usage:
%
%   Cout = GrB.apply (op, A, desc)
%   Cout = GrB.apply (Cin, accum, op, A, desc)
%   Cout = GrB.apply (Cin, M, op, A, desc)
%   Cout = GrB.apply (Cin, M, accum, op, A, desc)
%
% GrB.apply applies a unary operator to the entries in the input matrix A,
% which may be a GraphBLAS or MATLAB matrix (sparse or full).  See 'help
% GrB.unopinfo' for a list of available unary operators.  Cout is returned
% as a GraphBLAS matrix, by default; see 'help GrB/descriptorinfo' for
% more options.
%
% The op and A arguments are required.
%
% accum: a binary operator to accumulate the results.
%
% Cin, and the mask matrix M, and the accum operator are optional.  If
% either accum or M is present, then Cin is a required input. If desc.in0
% is 'transpose' then A is transposed before applying the operator, as
% C<M> = accum (C, f(A')) where f(...) is the unary operator.
%
% See also spfun.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[args, is_gb] = gb_get_args (varargin {:}) ;
if (is_gb)
    Cout = GrB (gbapply (args {:})) ;
else
    Cout = gbapply (args {:}) ;
end

