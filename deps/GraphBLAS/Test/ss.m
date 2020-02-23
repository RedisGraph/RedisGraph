% test GxB_select

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

clear all
make

for k = 32 % [1 2 4 8 16 32]
    nthreads_set (k)
    debug_off
    grbinfo
    test27  % band, for user-defined
    debug_on
    grbinfo
    test27
end

