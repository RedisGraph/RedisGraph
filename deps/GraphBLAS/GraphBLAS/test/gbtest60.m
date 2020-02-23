function gbtest60
%GBTEST60 test GrB.issigned

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

signed_types   = { 'double', 'single', 'int8', 'int16', 'int32', 'int64' } ;
unsigned_types = { 'logical', 'uint8', 'uint16', 'uint32', 'uint64' } ;

for k = 1:length (signed_types)
    type = signed_types {k} ;
    assert (GrB.issigned (type)) ;
end

for k = 1:length (unsigned_types)
    type = unsigned_types {k} ;
    assert (~GrB.issigned (type)) ;
end

fprintf ('gbtest60: all tests passed\n') ;

