function C = zeros (varargin)
%ZEROS a matrix with no entries.
%
%   C = zeros (n) ;      n-by-n GrB double matrix with no entries.
%   C = zeros (m,n) ;    m-by-n GrB double matrix with no entries.
%   C = zeros ([m,n]) ;  m-by-n GrB double matrix with no entries.
%   C = zeros (..., type) ;      empty matrix of given type.
%   C = zeros (..., 'like', G) ; empty matrix, same type as G.
%
% Since function overloads the MATLAB built-in zeros(...), at least one
% input must be a GraphBLAS matrix to use this version (for example,
% C = zeros (GrB (n))).  Alternatively, C = GrB (n,n) can be used.
%
% See also GrB/ones, GrB/false, GrB/true.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

[m, n, type] = gb_parse_args ('zeros', varargin {:}) ;
C = GrB (gbnew (m, n, type)) ;

