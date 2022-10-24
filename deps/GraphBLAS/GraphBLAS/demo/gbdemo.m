%GBDEMO run the graphblas_demo.m

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

if (exist ('echodemo'))
    % MATLAB: use the built-in echodemo to run the demo
    echodemo ('graphblas_demo') ;
else
    % Octave might not yet implement the echodemo function
    graphblas_demo
end


