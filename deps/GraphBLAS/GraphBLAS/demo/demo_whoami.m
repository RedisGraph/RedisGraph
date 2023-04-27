function result = demo_whoami
%DEMO_WHOAMI return 'Octave' or 'MATLAB'

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

if (demo_octave)
    result = 'Octave' ;
else
    result = 'MATLAB' ;
end

