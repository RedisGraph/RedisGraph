function axb_method (addop, multop, add, mult, ztype, xytype, identity, ...
    handle_flipxy)
%AXB_METHOD create a function to compute C=A*B over a semiring
%
% axb_method (addop, multop, add, mult, ztype, xytype, identity, handle_flipxy)

if (nargin < 8)
    handle_flipxy = 0 ;
end

f = fopen ('control.m4', 'w') ;

switch (xytype)
    case 'bool'
        fname = 'bool' ;
    case 'int8_t'
        fname = 'int8' ;
    case 'uint8_t'
        fname = 'uint8' ;
    case 'int16_t'
        fname = 'int16' ;
    case 'uint16_t'
        fname = 'uint16' ;
    case 'int32_t'
        fname = 'int32' ;
    case 'uint32_t'
        fname = 'uint32' ;
    case 'int64_t'
        fname = 'int64' ;
    case 'uint64_t'
        fname = 'uint64' ;
    case 'float'
        fname = 'fp32' ;
    case 'double'
        fname = 'fp64' ;
end

name = sprintf ('%s_%s_%s', addop, multop, fname) ;

fprintf (f, 'define(`GB_AgusB'', `GB_AgusB__%s'')\n', name) ;
fprintf (f, 'define(`GB_AdotB'', `GB_AdotB__%s'')\n', name) ;
fprintf (f, 'define(`GB_AheapB'', `GB_AheapB__%s'')\n', name) ;
fprintf (f, 'define(`GB_ztype'', `%s'')\n', ztype) ;
fprintf (f, 'define(`GB_xtype'', `%s'')\n', xytype) ;
fprintf (f, 'define(`GB_ytype'', `%s'')\n', xytype) ;
fprintf (f, 'define(`GB_identity'', `%s'')\n', identity) ;

% if handle_flipxy is true, then mult(x,y) is not commutative,
% and the types of x and y may also differ
fprintf (f, 'define(`GB_handle_flipxy'', %d)\n', handle_flipxy) ;

mult = strrep (mult, 'x', '`$2''') ;
mult = strrep (mult, 'y', '`$3''') ;
fprintf (f, 'define(`GB_MULT'', `$1 = %s'')\n', mult) ;

add = strrep (add, 'w', '`$1''') ;
add = strrep (add, 't', '`$2''') ;
fprintf (f, 'define(`GB_ADD'', `%s'')\n', add) ;

fclose (f) ;
% type control.m4

cmd = sprintf (...
'cat control.m4 Generator/GB_AxB.c | m4 | tail +8 > Generated/GB_AxB__%s.c', ...
name) ;
fprintf ('%s\n', cmd) ;
system (cmd) ;

cmd = sprintf (...
'cat control.m4 Generator/GB_AxB.h | m4 | tail +8 >> Generated/GB_AxB__semirings.h') ;
% fprintf ('%s\n', cmd) ;
system (cmd) ;

delete ('control.m4') ;

