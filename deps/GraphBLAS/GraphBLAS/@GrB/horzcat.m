function C = horzcat (varargin)
%HORZCAT horizontal concatenation.
% [A B] or [A,B] is the horizontal concatenation of A and B.
% Multiple matrices may be concatenated, as [A, B, C, ...].
% If the matrices have different types, the type is determined
% according to the results in GrB.optype.
%
% See also GrB/vertcat, GrB.optype.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

% FUTURE: this will be much faster when it is a mexFunction.

% determine the size of each matrix and the size of the result
nmatrices = length (varargin) ;
nvals = zeros (1, nmatrices) ;
ncols = zeros (1, nmatrices) ;
A = varargin {1} ;
if (isobject (A))
    A = A.opaque ;
end
[m, n, type] = gbsize (A) ;
nvals (1) = gbnvals (A) ;
ncols (1) = n ;
clear A
for k = 2:nmatrices
    A = varargin {k} ;
    if (isobject (A))
        A = A.opaque ;
    end
    [m2, n2, type2] = gbsize (A) ;
    if (m ~= m2)
        error ('Dimensions of arrays not consistent') ;
    end
    nvals (k) = gbnvals (A) ;
    ncols (k) = n2 ;
    type = gboptype (type, type2) ;
    clear A ;
end
ncols = [0 cumsum(ncols)] ;
nvals = [0 cumsum(nvals)] ;
cnvals = nvals (end) ;
n = ncols (end) ;

% allocate the I,J,X arrays
I = zeros (cnvals, 1, 'int64') ;
J = zeros (cnvals, 1, 'int64') ;
X = zeros (cnvals, 1, type) ;

% fill the I,J,X arrays
desc.base = 'zero-based' ;
for k = 1:nmatrices
    A = varargin {k} ;
    if (isobject (A))
        A = A.opaque ;
    end
    [i, j, x] = gbextracttuples (A, desc) ;
    noffset = int64 (ncols (k)) ;
    k1 = nvals (k) + 1 ;
    k2 = nvals (k+1) ;
    I (k1:k2) = i ;
    J (k1:k2) = j + noffset ;
    X (k1:k2) = x ;
end

% build the output matrix
C = GrB (gbbuild (I, J, X, m, n, desc)) ;

