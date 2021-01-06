function C = ones (varargin)
%ONES a matrix with all ones.
%
%   C = ones (n) ;      n-by-n GrB double matrix of all ones.
%   C = ones (m,n) ;    m-by-n GrB double matrix of all ones.
%   C = ones ([m,n]) ;  m-by-n GrB double matrix of all ones.
%   C = ones (..., type) ;      matrix of all ones of given type.
%   C = ones (..., 'like', G) ; matrix of all ones, same type as G.
%
% Since function overloads the MATLAB built-in ones(...), at least one
% input must be a GraphBLAS matrix to use this version; for example,
% C = ones (GrB (n), 'int8').
%
% See also GrB/zeros, GrB/false, GrB/true.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

[m, n, type] = gb_parse_args ('ones', varargin {:}) ;
C = GrB (gb_scalar_to_full (m, n, type, gbformat, 1)) ;

