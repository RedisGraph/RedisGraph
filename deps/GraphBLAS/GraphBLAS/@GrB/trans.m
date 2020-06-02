function Cout = trans (varargin)
%GRB.TRANS transpose a sparse matrix.
%
% Usage:
%
%   Cout = GrB.trans (A, desc)
%   Cout = GrB.trans (Cin, accum, A, desc)
%   Cout = GrB.trans (Cin, M, A, desc)
%   Cout = GrB.trans (Cin, M, accum, A, desc)
%
% The descriptor is optional.  If desc.in0 is 'transpose', then C<M>=A or
% C<M>=accum(C,A) is computed, since the default behavior is to transpose
% the input matrix.
%
% All input matrices may be either GraphBLAS and/or MATLAB matrices, in
% any combination.  Cout is returned as a GraphBLAS matrix, by default;
% see 'help GrB/descriptorinfo' for more options.
%
% See also transpose, ctranspose.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[args, is_gb] = gb_get_args (varargin {:}) ;
if (is_gb)
    Cout = GrB (gbtrans (args {:})) ;
else
    Cout = gbtrans (args {:}) ;
end

