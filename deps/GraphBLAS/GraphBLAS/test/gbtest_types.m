function types = gbtest_types
%GBTEST_TYPES return a cell array of strings, listing all types
%
% See also gbtest_binops.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

types = {
    'double'
    'single'
    'logical'
    'int8'
    'int16'
    'int32'
    'int64'
    'uint8'
    'uint16'
    'uint32'
    'uint64'
    } ;

% FUTURE: add 'complex'

