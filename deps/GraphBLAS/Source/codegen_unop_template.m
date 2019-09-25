function codegen_unop_template (unop, bfunc, ifunc, ufunc, ffunc, dfunc)
%CODEGEN_UNOP_TEMPLATE create unop functions
%
% Generate functions for a unary operator, for all types.

fprintf ('\n%-7s', unop) ;

types = { 
'bool',
'int8_t',
'int16_t',
'int32_t',
'int64_t',
'uint8_t',
'uint16_t',
'uint32_t',
'uint64_t',
'float',
'double' } ;

bits = [ 
8
8
16
32
64
8
16
32
64
32
64 ] ;

ntypes = length (types) ;

for code1 = 1:ntypes
    ctype = types {code1} ;
    cbits = bits (code1) ;

    % determine the function
    if (ctype (1) == 'b')
        func = bfunc ;
    elseif (ctype (1) == 'i')
        func = ifunc ;
    elseif (ctype (1) == 'u')
        func = ufunc ;
    elseif (ctype (1) == 'f')
        func = ffunc ;
    elseif (ctype (1) == 'd')
        func = dfunc ;
    end

    for code2 = 1:ntypes
        atype = types {code2} ;

        % determine the casting function
        fcast = 'GB_ctype zarg = (GB_ctype) xarg' ;
        if (atype (1) == 'f' || atype (1) == 'd')
            if (ctype (1) == 'i')
                % casting float or double to int
                fcast = sprintf (...
                    'GB_ctype zarg ; GB_CAST_SIGNED(zarg,xarg,%d)', cbits) ;
            elseif (ctype (1) == 'u')
                % casting float or double to uint
                fcast = sprintf (...
                    'GB_ctype zarg ; GB_CAST_UNSIGNED(zarg,xarg,%d)', cbits) ;
            end
        end

        if (isequal (unop, 'one') && (code1 ~= code2))
            % do not create this operator
            continue ;
        end

        codegen_unop_method (unop, func, fcast, ctype, atype) ;
    end
end

