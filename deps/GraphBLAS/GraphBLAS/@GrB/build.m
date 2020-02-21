function C = build (varargin)
%GRB.BUILD construct a GraphBLAS sparse matrix from a list of entries.
%
% Usage
%
%   C = GrB.build (I, J, X, m, n, dup, type, desc)
%
% GrB.build constructs an m-by-n GraphBLAS sparse matrix C from a list of
% entries, analogous to A = sparse (I, J, X, m, n) to construct a MATLAB
% sparse matrix A.
%
% If not present or empty, m defaults to the largest row index in the
% list I, and n defaults to the largest column index in the list J.  dup
% defaults to '+', which gives the same behavior as the MATLAB sparse
% function: duplicate entries are added together.
%
% dup is a string that defines a binary function; see 'help GrB.binopinfo'
% for a list of available binary operators.  The dup operator need not be
% associative.  If two entries in [I,J,X] have the same row and column
% index, the dup operator is applied to assemble them into a single
% entry.  Suppose (i,j,x1), (i,j,x2), and (i,j,x3) appear in that order
% in [I,J,X], in any location (the arrays [I J] need not be sorted, and
% so these entries need not be adjacent).  That is, i = I(k1) = I(k2) =
% I(k3) and j = J(k1) = J(k2) = J(k3) for some k1 < k2 < k3.  Then C(i,j)
% is computed as follows, in order:
%
%   x = X (k1) ;
%   x = dup (x, X (k2)) ;
%   x = dup (x, X (k3)) ;
%   C (i,j) = x ;
%
% For example, if the dup operator is '1st', then C(i,j)=X(k1) is set,
% and the subsequent entries are ignored.  If dup is '2nd', then
% C(i,j)=X(k3), and the preceding entries are ignored.
%
% type is a string that defines the type of C (see 'help GrB' for a list
% of types).  The type need not be the same type as the dup operator
% (unless one has a type of 'complex', in which case both must be
% 'complex').  If the type is not specified, it defaults to the type of
% X.
%
% The integer arrays I and J may be double, int64, or uint64:
% If I, J, and X are double, the following examples construct the same
% MATLAB sparse matrix S:
%
%   S = sparse (I, J, X) ;
%   S = GrB.build (I, J, X, struct ('kind', 'sparse')) ;
%   S = double (GrB.build (I, J, X)) ;
%   S = double (GrB.build (uint64(I), uint64(J), X)) ;
%
% I and J need not be in any particular order, but GrB.build is fastest if I
% and J are provided in column-major order if building a MATLAB sparse
% matrix.  If desc.format is 'by row', then GrB.build is fastest if I and
% J are in row-major order.
%
% If desc.base is 'zero-based', then I and J are treated as zero-based,
% where (0,0) is the first entry in the top left of S, and (m-1,n-1)
% is the position in the bottom right corner of S.  GrB.build is fastest
% if I and J are int64 or uint64, and desc.base is 'zero-based'.  The
% default is the same as MATLAB, which is 'one-based'.
%
% If I, J, and/or X are scalars, and any of I, J, or X is a vector of
% length e, the scalars are expanded into vectors of length e.  Any
% vectors I, J, and X must all be the same size, e.
%
% See also sparse, find, GrB.extracttuples.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[args, is_gb] = gb_get_args (varargin {:}) ;
if (is_gb)
    C = GrB (gbbuild (args {:})) ;
else
    C = gbbuild (args {:}) ;
end

