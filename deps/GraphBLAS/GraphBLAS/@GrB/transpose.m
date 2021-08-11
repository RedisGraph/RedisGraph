function C = transpose (G)
%TRANSPOSE C = G.', array transpose.
% C = G.' is the array transpose of G.
%
% See also GrB.trans, GrB/ctranspose.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

G = G.opaque ;
C = GrB (gbtrans (G)) ;

