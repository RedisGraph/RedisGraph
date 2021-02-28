function x = gb_get_scalar (A)
%GB_GET_SCALAR get the first scalar from a matrix

[~, ~, x] = GrB.extracttuples (A) ;
if (isempty (x))
    x = 0 ;
else
    x = x (1) ;
end

