function C = gb_spones (G, type)
%GB_SPONES return pattern of GraphBLAS matrix.
% Implements C = spones (G).

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

if (nargin == 1)
    switch (gbtype (G))
        case { 'single complex' }
            op = '1.single' ;
        case { 'double complex' }
            op = '1.double' ;
        otherwise
            op = '1' ;
    end
else
    if (~ischar (type))
        error ('type must be a string') ;
    end
    op = ['1.' type] ;
end

C = gbapply (op, G) ;

