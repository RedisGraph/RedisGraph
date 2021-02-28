function C = false (varargin)
%FALSE an all-false GraphBLAS matrix.
% C = false (m, n, 'like', G) or C = false ([m n], 'like', G) constructs
% a logical GraphBLAS matrix of size m-by-n with no entries.
%
% See also ones, true, zeros.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

arg1 = varargin {1} ;
if (length (arg1) == 2)
    m = arg1 (1) ;
    n = arg1 (2) ;
else
    m = arg1 ;
    n = varargin {2} ;
end

C = GrB (m, n, 'logical') ;

