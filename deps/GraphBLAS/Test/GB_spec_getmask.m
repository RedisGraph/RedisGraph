function Mask = GB_spec_getmask (Mask, Mask_struct)
%GB_SPEC_GETMASK return the mask, typecasted to logical

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (isstruct (Mask))
    if (Mask_struct)
        if (isfield (Mask, 'pattern'))
            Mask = Mask.pattern ;
        elseif (issparse (Mask))
            Mask = GB_spones_mex (Mask) ;
        end
    else
        Mask = Mask.matrix ;
    end
else
    if (Mask_struct && issparse (Mask))
        Mask = GB_spones_mex (Mask) ;
    end
end

Mask = GB_mex_cast (full (Mask), 'logical') ;

