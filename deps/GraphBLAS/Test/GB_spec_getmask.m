function Mask = GB_spec_getmask (Mask)
%GB_SPEC_GETMASK return the mask, typecasted to logical

if (isstruct (Mask))
    Mask = Mask.matrix ;
end

Mask = GB_mex_cast (full (Mask), 'logical') ;

