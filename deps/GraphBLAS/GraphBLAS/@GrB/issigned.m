function s = issigned (type)
%GRB.ISSIGNED Determine if a type is signed or unsigned.
% s = GrB.issigned (type) returns true if type is the string 'double',
% 'single', 'int8', 'int16', 'int32', or 'int64'.
%
% See also isinteger, isreal, isnumeric, isfloat.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

s = isequal (type, 'double') || isequal (type, 'single') || ...
    isequal (type, 'int8')   || isequal (type, 'int16')  || ...
    isequal (type, 'int32')  || isequal (type, 'int64') ;

