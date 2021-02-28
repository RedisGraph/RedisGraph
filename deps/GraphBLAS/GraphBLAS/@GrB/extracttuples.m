function [I,J,X] = extracttuples (varargin)
%GRB.EXTRACTTUPLES extract a list of entries from a matrix.
%
% Usage:
%
%   [I,J,X] = GrB.extracttuples (A, desc)
%
% GrB.extracttuples extracts all entries from either a MATLAB matrix or a
% GraphBLAS matrix.  If A is a MATLAB sparse or dense matrix, [I,J,X] =
% GrB.extracttuples (A) is identical to [I,J,X] = find (A).
%
% For a GraphBLAS matrix G, GrB.extracttuples (G) returns any explicit
% zero entries in G, while find (G) excludes them.
%
% The descriptor is optional.  desc.base is a string, eithe 'default',
% 'zero-based', 'one-based int', or 'one-based'.  This parameter
% determines the type of output for I and J.  The default is one-based,
% so that I and J are returned as double vectors, with one-based indices.
% If max(size(A)) > flintmax, however, the default is 'one-based int', so
% that I and J are int64 vectors with one-based indices.  One-based
% indices in I are in the range 1 to m, and the indices in J are in the
% range 1 to n, if A is m-by-n.  This is identical to [I,J,X] = find (A)
% for a MATLAB sparse or dense matrix.
%
% If 'zero-based', I and J are returned as int64 arrays, with zero-based
% indices.  The entries in I and J are in the range 0 to m-1 and 0 to
% n-1, respectively, if [m n] = size (A).  This usage is not the
% conventional 1-based indexing in MATLAB, but it is the fastest method.
%
% The overloaded [I,J,X] = find (A) method for a GraphBLAS matrix A uses
% desc.base of 'default'.
%
% This function corresponds to the GrB_*_extractTuples_* functions in
% GraphBLAS.
%
% See also find, GrB/build.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[args, ~] = gb_get_args (varargin {:}) ;
if (nargout == 3)
    [I, J, X] = gbextracttuples (args {:}) ;
elseif (nargout == 2)
    [I, J] = gbextracttuples (args {:}) ;
else
    I = gbextracttuples (args {:}) ;
end

