function f = gb_fmt (A)
%GB_FMT return the format of A as a single string.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

[f, s] = GrB.format (A) ;

if (~isempty (s))
    f = [s ' ' f] ;
end

