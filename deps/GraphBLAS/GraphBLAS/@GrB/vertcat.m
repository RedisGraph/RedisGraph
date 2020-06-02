function C = vertcat (varargin)
%VERTCAT Vertical concatenation.
% [A ; B] is the vertical concatenation of A and B.
% A and B may be GraphBLAS or MATLAB matrices, in any combination.
% Multiple matrices may be concatenated, as [A ; B ; C ; ...].
%
% See also horzcat, GrB/horzcat.

% FUTURE: this will be much faster when it is a mexFunction.
% The version below requires a sort in GrB.build.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% determine the size of each matrix and the size of the result
nmatrices = length (varargin) ;
nvals = zeros (1, nmatrices) ;
nrows = zeros (1, nmatrices) ;
A = varargin {1} ;
[m, n] = size (A) ;
nvals (1) = GrB.entries (A) ;
nrows (1) = m ;
type = GrB.type (A) ;
clear A
for k = 2:nmatrices
    B = varargin {k} ;
    [m, n2] = size (B) ;
    if (n ~= n2)
        gb_error ('Dimensions of arrays being concatenated are not consistent');
    end
    nvals (k) = GrB.entries (B) ;
    nrows (k) = m ;
    clear B ;
end
nrows = [0 cumsum(nrows)] ;
nvals = [0 cumsum(nvals)] ;
cnvals = nvals (end) ;
m = nrows (end) ;

% allocate the I,J,X arrays
I = zeros (cnvals, 1, 'int64') ;
J = zeros (cnvals, 1, 'int64') ;
X = zeros (cnvals, 1, type) ;

% fill the I,J,X arrays
desc.base = 'zero-based' ;
for k = 1:nmatrices
    [i, j, x] = GrB.extracttuples (varargin {k}, desc) ;
    moffset = int64 (nrows (k)) ;
    koffset = nvals (k) ;
    kvals = GrB.entries (varargin {k}) ;
    I ((koffset+1):(koffset+kvals)) = i + moffset ;
    J ((koffset+1):(koffset+kvals)) = j ;
    X ((koffset+1):(koffset+kvals)) = x ;
end

% build the output matrix
C = GrB.build (I, J, X, m, n, desc) ;

