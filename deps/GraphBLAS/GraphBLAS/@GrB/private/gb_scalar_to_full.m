function C = gb_scalar_to_full (m, n, type, fmt, scalar)
%GB_SCALAR_TO_FULL expand a scalar into a full matrix

C = gbsubassign (gbnew (m, n, type, fmt), gbfull (scalar)) ;

