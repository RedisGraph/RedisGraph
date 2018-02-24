function axb_method (addop, multop, add, mult, ztype, xytype, identity)
%AXB_METHOD create a function to compute C=A*B over a semiring
%
% axb_method (addop, multop, add, mult, ztype, xytype, identity)

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

fprintf (f, 'define(`GB_AxB'', `GB_AxB__%s'')\n', name) ;
fprintf (f, 'define(`GB_AdotB'', `GB_AdotB__%s'')\n', name) ;
fprintf (f, 'define(`ztype'', `%s'')\n', ztype) ;
fprintf (f, 'define(`xytype'', `%s'')\n', xytype) ;
fprintf (f, 'define(`identity'', `%s'')\n', identity) ;

mult = strrep (mult, 'x', '`$1''') ;
mult = strrep (mult, 'y', '`$2''') ;
fprintf (f, 'define(`MULT'', `%s'')\n', mult) ;

add = strrep (add, 'w', '`$1''') ;
add = strrep (add, 't', '`$2''') ;
fprintf (f, 'define(`ADD'', `%s'')\n', add) ;

fclose (f) ;
% type control.m4

cmd = sprintf (...
'cat control.m4 Template/GB_AxB.c | m4 | tail +8 > Generated/GB_AxB__%s.c', ...
name) ;
fprintf ('%s\n', cmd) ;
system (cmd) ;

cmd = sprintf (...
'cat control.m4 Template/GB_AxB.h | m4 | tail +8 >> Generated/GB_AxB__semirings.h') ;
% fprintf ('%s\n', cmd) ;
system (cmd) ;

delete ('control.m4') ;

