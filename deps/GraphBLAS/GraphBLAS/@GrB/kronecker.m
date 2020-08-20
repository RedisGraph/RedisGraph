function Cout = kronecker (varargin)
%GRB.KRONECKER sparse Kronecker product.
%
% Usage:
%
%   Cout = GrB.kronecker (op, A, B, desc)
%   Cout = GrB.kronecker (Cin, accum, op, A, B, desc)
%   Cout = GrB.kronecker (Cin, M, op, A, B, desc)
%   Cout = GrB.kronecker (Cin, M, accum, op, A, B, desc)
%
% GrB.kronecker computes the Kronecker product, T=kron(A,B), using the given
% binary operator op, in place of the conventional '*' operator for the
% MATLAB built-in kron.  See also C = kron (A,B), which uses the default
% semiring operators if A and/or B are GrB matrices.
%
% All input matrices may be either GraphBLAS and/or MATLAB matrices, in
% any combination.  Cout is returned as a GraphBLAS matrix, by default;
% see 'help GrB/descriptorinfo' for more options.
%
% T is then accumulated into C via C<#M,replace> = accum (C,T).
%
% See also kron, GrB/kron.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[args, is_gb] = gb_get_args (varargin {:}) ;
if (is_gb)
    Cout = GrB (gbkronecker (args {:})) ;
else
    Cout = gbkronecker (args {:}) ;
end

