function codegen
%CODEGEN generate all code for Generated1 and Generated2
%
% This code generation method works on octave7 and MATLAB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

codegen_1type ;     % types
codegen_axb ;       % semirings
codegen_binop ;     % binary operators
codegen_unop ;      % unary operators
codegen_red ;       % monoids
codegen_sel ;       % select operators


