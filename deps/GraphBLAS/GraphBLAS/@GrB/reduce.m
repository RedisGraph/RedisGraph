function Cout = reduce (varargin)
%GRB.REDUCE reduce a matrix to a scalar.
%
% Usage:
%
%   cout = GrB.reduce (monoid, A)
%   cout = GrB.reduce (monoid, A, desc)
%   cout = GrB.reduce (cin, accum, monoid, A)
%   cout = GrB.reduce (cin, accum, monoid, A, desc)
%
% GrB.reduce reduces a matrix to a scalar, using the given monoid.  The
% valid monoids are: '+', '*', 'max', and 'min' for all but the 'logical'
% type, and '|', '&', 'xor', and 'ne' for the 'logical' type.  See 'help
% GrB.monoidinfo' for more details.
%
% The monoid and A arguments are required.  All others are optional.  The
% op is applied to all entries of the matrix A to reduce them to a single
% scalar result.
%
% accum: an optional binary operator (see 'help GrB.binopinfo' for a
% list).
%
% cin: an optional input scalar into which the result can be accumulated
% with cout = accum (cin, result).
%
% All input matrices may be either GraphBLAS and/or MATLAB matrices, in
% any combination.  cout is returned as a GraphBLAS scalar, by default;
% see 'help GrB/descriptorinfo' for more options.
%
% See also GrB.vreduce; sum, prod, max, min.

% FUTURE: add complex monoids.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[args, is_gb] = gb_get_args (varargin {:}) ;
if (is_gb)
    Cout = GrB (gbreduce (args {:})) ;
else
    Cout = gbreduce (args {:}) ;
end

