function C = true (varargin)
%TRUE a GraphBLAS logical matrix with all true values.
% C = true (m, n, 'like', G) or C = ones ([m n], 'like', G) constructs an
% m-by-n GraphBLAS logical matrix C with all entries equal to true.
%
% See also zeros, false, true.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = GrB.subassign (false (varargin {:}), true) ;

