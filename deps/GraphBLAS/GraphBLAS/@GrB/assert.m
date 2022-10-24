function assert (G)
%ASSERT generate an error when a condition is violated.
%
% See also error.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

builtin ('assert', logical (G)) ;

