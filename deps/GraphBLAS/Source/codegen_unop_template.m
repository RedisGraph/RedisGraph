function codegen_unop_template (unop, bfunc, ifunc, ufunc, ffunc, dfunc, ...
    fcfunc, dcfunc)
%CODEGEN_UNOP_TEMPLATE create unop functions
%
% codegen_unop_template (unop, bfunc, ifunc, ufunc, ffunc, dfunc, ...
%       fcfunc, dcfunc)
%
%       unop:   operator name
%
%   strings defining each function, or empty if no such unary operator:
%
%       bfunc:  bool
%       ifunc:  int8, int16, int32, int64
%       ufunc:  uint8, uint16, uint32, uint64
%       ffunc:  float
%       dfunc:  double
%       fcfunc: GxB_FC32_t
%       dcfunc: GxB_FC64_t
%
% Generate functions for a unary operator, for all types.
%
% The 'identity' operator is unique: it is used for typecasting, and all 13*13
% pairs of functions are generated.  The other unary operators are defined only
% for a single type (ctype == atype).

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

fprintf ('\n%-9s', unop) ;

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
'double',
'GxB_FC32_t',
'GxB_FC64_t' } ;

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
64
64
128 ] ;

ntypes = length (types) ;

for code1 = 1:ntypes
    ctype = types {code1} ;
    cbits = bits (code1) ;

    % determine the function
    if (isequal (ctype, 'bool')) 
        func = bfunc ;
    elseif (ctype (1) == 'i')
        func = ifunc ;
    elseif (ctype (1) == 'u')
        func = ufunc ;
    elseif (isequal (ctype, 'float')) 
        func = ffunc ;
    elseif (isequal (ctype, 'double')) 
        func = dfunc ;
    elseif (isequal (ctype, 'GxB_FC32_t')) 
        func = fcfunc ;
    elseif (isequal (ctype, 'GxB_FC64_t')) 
        func = dcfunc ;
    end

    if (isempty (func))
        % skip this operator
        continue ;
    end

    if (isequal (unop, 'identity'))
        % create identity operators for all pairs of types
        code2s = 1:ntypes ;
    else
        % create operators only for code1 == code2
        code2s = code1 ;
    end

    for code2 = code2s
        atype = types {code2} ;

        % determine the casting function
        fcast = 'GB_ctype zarg = (GB_ctype) xarg' ;

        if (isequal (atype, ctype))

            % no typecasting
            fcast = 'GB_ctype zarg = xarg' ;

        elseif (isequal (atype, 'GxB_FC32_t'))

            % typecasting from GxB_FC32_t
            if (isequal (ctype, 'bool')) 
                % to bool from GxB_FC32_t 
                fcast = 'GB_ctype zarg = (crealf (xarg) != 0) || (cimagf (xarg) != 0)' ;
            elseif (codegen_contains (ctype, 'int'))
                % to integer from GxB_FC32_t 
                fcast = sprintf ('GB_ctype zarg = GB_cast_to_%s ((double) crealf (xarg))', ctype) ;
            elseif (isequal (ctype, 'float') || isequal (ctype, 'double')) 
                % to float or double from GxB_FC32_t 
                fcast = 'GB_ctype zarg = (GB_ctype) crealf (xarg)' ;
            elseif (isequal (ctype, 'GxB_FC64_t'))
                % to GxB_FC64_t from GxB_FC32_t 
                fcast = 'GB_ctype zarg = GxB_CMPLX ((double) crealf (xarg), (double) cimagf (xarg))' ;
            end

        elseif (isequal (atype, 'GxB_FC64_t'))

            % typecasting from GxB_FC64_t
            if (isequal (ctype, 'bool')) 
                % to bool from GxB_FC64_t 
                fcast = 'GB_ctype zarg = (creal (xarg) != 0) || (cimag (xarg) != 0)' ;
            elseif (codegen_contains (ctype, 'int'))
                % to integer from GxB_FC64_t 
                fcast = sprintf ('GB_ctype zarg = GB_cast_to_%s (creal (xarg))', ctype) ;
            elseif (isequal (ctype, 'float') || isequal (ctype, 'double')) 
                % to float or double from GxB_FC64_t 
                fcast = 'GB_ctype zarg = (GB_ctype) creal (xarg)' ;
            elseif (isequal (ctype, 'GxB_FC32_t'))
                % to GxB_FC32_t from GxB_FC64_t 
                fcast = 'GB_ctype zarg = GxB_CMPLXF ((float) creal (xarg), (float) cimag (xarg))' ;
            end

        elseif (isequal (atype, 'float') || isequal (atype, 'double'))

            % typecasting from float or double
            if (isequal (ctype, 'bool')) 
                % to bool from float or double 
                fcast = 'GB_ctype zarg = (xarg != 0)' ;
            elseif (codegen_contains (ctype, 'int'))
                % to integer from float or double 
                fcast = sprintf ('GB_ctype zarg = GB_cast_to_%s ((double) (xarg))', ctype) ;
            elseif (isequal (ctype, 'GxB_FC32_t'))
                % to GxB_FC32_t from float or double
                fcast = 'GB_ctype zarg = GxB_CMPLXF ((float) (xarg), 0)' ;
            elseif (isequal (ctype, 'GxB_FC64_t'))
                % to GxB_FC64_t from float or double
                fcast = 'GB_ctype zarg = GxB_CMPLX ((double) (xarg), 0)' ;
            end

        elseif (isequal (ctype, 'GxB_FC32_t'))

            % typecasting to GxB_FC32_t to any real type
            fcast = 'GB_ctype zarg = GxB_CMPLXF ((float) (xarg), 0)' ;

        elseif (isequal (ctype, 'GxB_FC64_t'))

            % typecasting to GxB_FC32_t to any real type
            fcast = 'GB_ctype zarg = GxB_CMPLX ((double) (xarg), 0)' ;

        end

        codegen_unop_method (unop, func, fcast, ctype, atype) ;
    end
end

