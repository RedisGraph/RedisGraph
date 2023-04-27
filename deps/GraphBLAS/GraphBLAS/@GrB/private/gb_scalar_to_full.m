function C = gb_scalar_to_full (m, n, type, fmt, scalar)
%GB_SCALAR_TO_FULL expand a scalar into a full matrix

if (~isempty (strfind (fmt, 'by row'))) %#ok<STREMP>
    fmt = 'by row' ;
else
    fmt = 'by col' ;
end

C = gbsubassign (gbnew (m, n, type, fmt), gbfull (scalar)) ;

