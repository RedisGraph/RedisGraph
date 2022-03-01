function result = demo_octave
%DEMO_OCTAVE return true if Octave is in use, false for MATLAB

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

result = (exist ('OCTAVE_VERSION', 'builtin') == 5) ;
