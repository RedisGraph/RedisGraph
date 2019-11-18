function C = ones (varargin)
%ONES a matrix with all ones, the same type as G.
% C = ones (m, n, 'like', G) or C = ones ([m n], 'like', G) constructs an
% m-by-n GraphBLAS matrix C with all entries equal to one.  C has the
% same type as G.
%
% See also zeros, false, true.

% FUTURE: GrB_assign and GxB_subassign need to have a special case for this,
% as the expansion of a scalar to a dense matrix.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = GrB.subassign (zeros (varargin {:}), 1) ;

