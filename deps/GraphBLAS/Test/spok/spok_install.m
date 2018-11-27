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

is64 = ~isempty (strfind (computer, '64')) ;
if (is64)
    fprintf ('Compiling spok (64-bit)\n') ;
    mex -largeArrayDims spok.c spok_mex.c
else
    fprintf ('Compiling spok (32-bit)\n') ;
    mex spok.c spok_mex.c
end
addpath (pwd) ;
