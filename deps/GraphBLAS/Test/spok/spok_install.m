function spok_install
%SPOK_INSTALL compiles and installs the SPOK mexFunction
% Your current working directory must be the "spok" directory for this function
% to work.
%
% Example:
%   spok_install
%
% See also sparse, spok, spok_test

% Copyright 2008-2011, Timothy A. Davis, http://www.suitesparse.com

mex -largeArrayDims spok.c spok_mex.c
addpath (pwd) ;
