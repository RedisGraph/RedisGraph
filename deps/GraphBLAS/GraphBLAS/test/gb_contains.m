function s = gb_contains (text, pattern)
%GB_CONTAINS same as contains (text, pattern)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

s = ~isempty (strfind (text, pattern)) ; %#ok<STREMP>
