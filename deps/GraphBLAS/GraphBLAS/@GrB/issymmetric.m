function s = issymmetric (G, option)
%ISSYMMETRIC Determine if a GraphBLAS matrix is real or complex symmetric.
% issymmetric (G) is true if G equals G.' and false otherwise.
% issymmetric (G, 'skew') is true if G equals -G.' and false otherwise.
% issymmetric (G, 'nonskew') is the same as issymmetric (G).
%
% See also GrB/ishermitian.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;

if (nargin < 2)
    option = 'nonskew' ;
end

s = gb_issymmetric (G, option, false) ;

