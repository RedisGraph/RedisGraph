function C = false (varargin)
%FALSE a logical matrix with no entries.
%
%   C = false (n) ;      n-by-n GrB logical matrix with no entries.
%   C = false (m,n) ;    m-by-n GrB logical matrix with no entries.
%   C = false ([m,n]) ;  m-by-n GrB logical matrix with no entries.
%   C = false (..., type) ;      empty logical matrix of given type.
%   C = false (..., 'like', G) ; empty logical matrix, same type as G.
%
% Since function overloads the MATLAB built-in false(...), at least one
% input must be a GraphBLAS matrix to use this version (for example,
% C = false (GrB (n))).  Alternatively, C = GrB (n,n,'logical') can be
% used instead.
%
% See also GrB/ones, GrB/true, GrB/zeros.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

[m, n, ~] = gb_parse_args ('false', varargin {:}) ;
C = GrB (gbnew (m, n, 'logical')) ;

