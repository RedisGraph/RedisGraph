function spok_install
%SPOK_INSTALL compiles and installs the SPOK mexFunction
% Your current working directory must be the "spok" directory for this function
% to work.
%
% Example:
%   spok_install
%
% See also sparse, spok, spok_test

% Copyright 2008-2011, Timothy A. Davis, http://suitesparse.com
% SPDX-License-Identifier: Apache-2.0

mex -largeArrayDims spok.c spok_mex.c
addpath (pwd) ;
