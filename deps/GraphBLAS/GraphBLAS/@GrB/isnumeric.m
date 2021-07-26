function s = isnumeric (G) %#ok<INUSD>
%ISNUMERIC always true for any GraphBLAS matrix.
% isnumeric (G) is always true for any GraphBLAS matrix G, including
% logical matrices, since those matrices can be operated on in any
% semiring, just like any other GraphBLAS matrix.
%
% See also GrB/isfloat, GrB/isreal, GrB/isinteger, GrB/islogical,
% GrB.type, GrB/isa, GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

s = true ;

