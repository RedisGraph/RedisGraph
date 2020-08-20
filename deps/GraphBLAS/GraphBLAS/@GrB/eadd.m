function Cout = eadd (varargin)
%GRB.EADD sparse matrix addition.
%
% Usage:
%
%   Cout = GrB.eadd (op, A, B, desc)
%   Cout = GrB.eadd (Cin, accum, op, A, B, desc)
%   Cout = GrB.eadd (Cin, M, op, A, B, desc)
%   Cout = GrB.eadd (Cin, M, accum, op, A, B, desc)
%
% GrB.eadd computes the element-wise 'addition' T=A+B.  The result T has
% the pattern of the union of A and B. The operator is used where A(i,j)
% and B(i,j) are present.  Otherwise the entries in A and B are copied
% directly into T:
%
%   if (A(i,j) and B(i,j) is present)
%       T(i,j) = op (A(i,j), B(i,j))
%   elseif (A(i,j) is present but B(i,j) is not)
%       T(i,j) = A(i,j)
%   elseif (B(i,j) is present but A(i,j) is not)
%       T(i,j) = B(i,j)
%
% T is then accumulated into C via C<#M,replace> = accum (C,T).
%
% Cin, M, accum, and the descriptor desc are the same as all other
% GrB.methods; see GrB.mxm and GrB.descriptorinfo for more details.  For the
% binary operator, see GrB.binopinfo.
%
% All input matrices may be either GraphBLAS and/or MATLAB matrices, in
% any combination.  Cout is returned as a GraphBLAS matrix, by default;
% see 'help GrB/descriptorinfo' for more options.
%
% See also GrB.emult.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[args, is_gb] = gb_get_args (varargin {:}) ;
if (is_gb)
    Cout = GrB (gbeadd (args {:})) ;
else
    Cout = gbeadd (args {:}) ;
end

