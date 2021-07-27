function [p, varargout] = dmperm (G)
%DMPERM Dulmage-Mendelsohn permutation.
% See 'help dmperm' for details.
%
% See also GrB/amd, GrB/colamd.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

[p, varargout{1:nargout-1}] = builtin ('dmperm', logical (G)) ;

