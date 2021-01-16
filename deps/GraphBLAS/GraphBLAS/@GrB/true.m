function C = true (varargin)
%TRUE a logical matrix with all true values.
%
%   C = true (n) ;      n-by-n GrB logical matrix of all true entries.
%   C = true (m,n) ;    m-by-n GrB logical matrix of all true entries.
%   C = true ([m,n]) ;  m-by-n GrB logical matrix of all true entries.
%   C = true (..., type) ;      matrix of all true entries of given type.
%   C = true (..., 'like', G) ; matrix of all true entries, same type as G.
%
% Since function overloads the MATLAB built-in true(...), at least one
% input must be a GraphBLAS matrix to use this version (for example,
% C = true (GrB (n))).
%
% See also GrB/zeros, GrB/ones, GrB/false.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

[m, n, ~] = gb_parse_args ('true', varargin {:}) ;
C = GrB (gb_scalar_to_full (m, n, 'logical', gbformat, true)) ;

