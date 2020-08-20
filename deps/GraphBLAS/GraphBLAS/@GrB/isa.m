function s = isa (G, classname)
%ISA Determine if a GraphBLAS matrix is of specific class.
%
% For any GraphBLAS matrix G, isa (G, 'GrB'), isa (G, 'numeric'), and isa
% (G, 'object') are always true.
%
% isa (G, 'float') is the same as isfloat (G), and is true if the GrB
% matrix G has type 'double', 'single', or 'complex'.
%
% isa (G, 'integer') is the same as isinteger (G), and is true if the GrB
% matrix G has type 'int8', 'int16', 'int32', 'int64', 'uint8', 'uint16',
% 'uint32', or 'uint64'.
%
% isa (G, classname) is true if the classname matches the type of G.
%
% See also GrB.type, isnumeric, islogical, ischar, iscell, isstruct,
% isfloat, isinteger, isobject, isjava, issparse, isreal, class.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (isequal (classname, 'GrB') || isequal (classname, 'numeric'))
    % all GraphBLAS matrices are numeric, and have class name 'GrB'
    s = true ;
elseif (isequal (classname, 'float'))
    % GraphBLAS double, single, and complex matrices are 'float'
    s = isfloat (G) ;
elseif (isequal (classname, 'integer'))
    % GraphBLAS int* and uint* matrices are 'integer'
    s = isinteger (G) ;
elseif (isequal (GrB.type (G), classname))
    % specific cases, such as isa (G, 'double')
    s = true ;
else
    % catch-all for cases not handled above
    s = builtin ('isa', G, classname) ;
end

