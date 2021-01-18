function C = gb_expand (scalar, S, type)
%GB_EXPAND expand a scalar into a GraphBLAS matrix.
% Implements C = GrB.expand (scalar, S, type).  This function assumes the
% first input is a scalar; the caller has checked this already.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

% typecast the scalar to the desired type
scalar = gbnew (scalar, type) ;

% expand the scalar into the pattern of S
C = gbapply2 (['1st.' type], scalar, S) ;

