function Cout = vreduce (varargin)
%GRB.REDUCE reduce a matrix to a vector.
%
% Usage:
%
%   Cout = GrB.vreduce (monoid, A)
%   Cout = GrB.vreduce (monoid, A, desc)
%   Cout = GrB.vreduce (Cin, M, monoid, A)
%   Cout = GrB.vreduce (Cin, M, monoid, A, desc)
%   Cout = GrB.vreduce (Cin, accum, monoid, A)
%   Cout = GrB.vreduce (Cin, accum, monoid, A, desc)
%   Cout = GrB.vreduce (Cin, M, accum, monoid, A)
%   Cout = GrB.vreduce (Cin, M, accum, monoid, A, desc)
%
% The monoid and A arguments are required.  All others are optional.  The
% valid monoids are: '+', '*', 'max', and 'min' for all but the 'logical'
% type, and '|', '&', 'xor', and 'ne' for the 'logical' type.  See 'help
% GrB.monoidinfo' for more details.
%
% By default, each row of A is reduced to a scalar.  If Cin is not
% present, Cout (i) = reduce (A (i,:)).  In this case, Cin and Cout are
% column vectors of size m-by-1, where A is m-by-n.  If desc.in0 is
% 'transpose', then A.' is reduced to a column vector; Cout (j) = reduce
% (A (:,j)).  In this case, Cin and Cout are column vectors of size
% n-by-1, if A is m-by-n.
%
% All input matrices may be either GraphBLAS and/or MATLAB matrices, in
% any combination.  Cout is returned as a GraphBLAS matrix, by default;
% see 'help GrB/descriptorinfo' for more options.
%
% See also GrB.reduce, sum, prod, max, min.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[args, is_gb] = gb_get_args (varargin {:}) ;
if (is_gb)
    Cout = GrB (gbvreduce (args {:})) ;
else
    Cout = gbvreduce (args {:}) ;
end

