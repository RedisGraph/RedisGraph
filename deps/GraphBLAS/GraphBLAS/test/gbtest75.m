function gbtest75
%GBTEST75 test bitshift

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

fprintf ('\ngbtest75: bitshift\n') ;

for b = -8:8
    fprintf ('.') ;
    for a = intmin ('int8') : intmax ('int8')
        c = bitshift (a, b) ;
        c2 = bitshift (GrB (a), GrB (b)) ;
        assert (isequal (c, c2)) ;
    end
end

fprintf ('\ngbtest75: all tests passed\n') ;

