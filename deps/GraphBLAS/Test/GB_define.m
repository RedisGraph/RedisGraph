function GB_define 
%GB_DEFINE create C source code for GraphBLAS.h

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

ints = {'int8','uint8', 'int16','uint16', 'int32','uint32', 'int64','uint64' } ;
floats = { 'fp32', 'fp64' } ;

nonbool = [ints floats] ;

numeric     = { 'min', 'max', 'plus', 'times', 'any' } ;
numeric_ids = {  inf , -inf ,  0    ,  1     , 999   } ;

boolean     = { 'lor', 'land', 'lxor', 'eq', 'any' } ;
boolean_ids = {  false, true,   false,  true, inf  } ;

%-------------------------------------------------------------------------------
% construct all built-in monoids for GraphBLAS.h
%-------------------------------------------------------------------------------

format compact
n = 0 ;

for k = 1:length (numeric)

    x2 = upper (numeric {k}) ;
    fprintf ('    // %s monoids:\n', x2) ;

    for i = 1:length (ints)
        cc = upper (ints {i}) ;
        name = sprintf ('GxB_%s_%s_MONOID,', x2, cc) ;
        fprintf ('    %-25s     // identity: ', name) ;
        id = numeric_ids {k} ;
        switch id
            case inf
                s = sprintf ('%s_MAX', cc) ;
            case -inf
                if (cc (1) == 'U')
                    s = sprintf ('0') ;
                else
                    s = sprintf ('%s_MIN', cc) ;
                end
            otherwise
                s = sprintf ('%d', id) ;
        end
        fprintf ('%s\n', s) ;
    end

    for i = 1:length (floats)
        cc = upper (floats {i}) ;
        name = sprintf ('GxB_%s_%s_MONOID,', x2, cc) ;
        fprintf ('    %-25s     // identity: ', name) ;
        id = numeric_ids {k} ;
        switch id
            case inf
                s = 'INFINITY' ;
            case -inf
                s = '-INFINITY' ;
            case 999
                s = '(any value)' ;
            otherwise
                s = sprintf ('%d', id) ;
        end
        fprintf ('%s\n', s) ;
    end
    fprintf ('\n') ;

end

fprintf ('    // Boolean monoids:\n') ;
for k = 1:length (boolean)
    bb = boolean {k} ;
    id = boolean_ids {k} ;
    if (isequal (bb, 'eq')) 
        op = sprintf ('GxB_%s_BOOL', upper (bb)) ;
    else
        op = sprintf ('GxB_%s', upper (bb)) ;
    end

    if (k == length (boolean))
        name = sprintf ('GxB_%s_MONOID ;', upper (bb)) ;
    else
        name = sprintf ('GxB_%s_MONOID,', upper (bb)) ;
    end
    fprintf ('    %-25s     // identity: ', name) ;

    if (id == true)
        fprintf ('true ') ;
    elseif (id == false)
        fprintf ('false') ;
    else
        fprintf ('(any value)') ;
    end

    fprintf ('\n') ;
end

%-------------------------------------------------------------------------------
% construct all semirings for GraphBLAS.h
%-------------------------------------------------------------------------------

fprintf ('\n') ;
fprintf ('\n') ;

% 1000: x,y,z all nonboolean
n = 0 ;
for mult = {'first', 'second', 'pair', 'min', 'max', ...            % 5
            'plus', 'minus', 'times', 'div', 'rdiv', 'rminus', ...  % 6
            'iseq', 'isne', 'isgt', 'islt', 'isge', 'isle', ...     % 6
            'lor', 'land', 'lxor', }                                % 3
    fmult = upper (mult {1}) ;
    fprintf ('\n') ;
    fprintf ('    // semirings with multiply op: z = %s (x,y), all types x,y,z the same:\n', fmult) ;
    for c = nonbool 
        fprintf ('    ') ;
        for add = { 'min', 'max', 'plus', 'times', 'any' }
            ad = upper (add {1}) ;
            n = n + 1 ;
            s = sprintf ('GxB_%s_%s_%s', ad, fmult, upper (c{1})) ;
            fprintf ('%-23s, ', s) ;
        end
        fprintf ('\n') ;
    end
end

% fprintf ('\n so far %d\n', n) ;
assert (n == 1000) ;
% pause


% 300: x,y nonboolean, z boolean: 6 * 4 * 10
for mult = { 'eq', 'ne', 'gt', 'lt', 'ge', 'le' }
    fmult = upper (mult {1}) ;
    fprintf ('\n') ;
    fprintf ('    // semirings with multiply op: z = %s (x,y), where z is boolean and x,y are given by the suffix:\n', fmult) ;
    for c = nonbool
        fprintf ('    ') ;
        for add = { 'Lor', 'Land', 'Lxor', 'eq', 'any' }
            ad = upper (add {1}) ;
            n = n + 1 ;
            s = sprintf ('GxB_%s_%s_%s', ad, fmult, upper (c{1})) ;
            fprintf ('%-23s, ', s) ;
        end
        fprintf ('\n') ;
    end
end

assert (n == 1300) ;

% 55: x,y,z all boolean: 11 * 5
fprintf ('\n') ;
fprintf ('    // purely boolean semirings (in the form GxB_(add monoid)_(multipy operator)_BOOL:\n') ;
for mult = { 'first', 'second', 'pair', 'Lor', 'Land', 'Lxor', ...
             'eq', 'gt', 'lt', 'ge', 'le' }
    fmult = upper (mult {1}) ;
    fprintf ('    ') ;
    for add = { 'Lor', 'Land', 'Lxor', 'eq', 'any' }
        ad = upper (add {1}) ;
        n = n + 1 ;
        s = sprintf ('GxB_%s_%s_BOOL', ad, fmult) ;
        fprintf ('%-23s, ', s) ;
    end
    fprintf ('\n') ;
end

fprintf ('semirings: %d\n', n) ;
assert (n == 1355) ;
pause


%-------------------------------------------------------------------------------
% construct all semirings for GB_mx_semiring.c
%-------------------------------------------------------------------------------

n = 0 ;

% 1000: x,y,z all nonboolean:  (8+6+3)*4*10

fprintf ('    if (zcode != GB_BOOL_code)\n') ;
fprintf ('    {\n') ;
fprintf ('        switch (mult_opcode)\n') ;
fprintf ('        {\n') ;
for mult = {'first', 'second', 'pair', 'min', 'max', ...
    'plus', 'minus', 'rminus', 'times', 'div', 'rdiv', ...
    'iseq', 'isne', 'isgt', 'islt', 'isge', 'isle', ...
    'lor', 'land', 'lxor', }
    fmult = upper (mult {1}) ;
    fprintf ('\n') ;
    fprintf ('            case GB_%s_opcode : // with (5 monoids) x (10 nonboolean types)\n\n', fmult) ;
    fprintf ('                switch (add_opcode)\n') ;
    fprintf ('                {\n') ;
    for add = { 'min', 'max', 'plus', 'times', 'any' }
        ad = upper (add {1}) ;
        fprintf ('\n') ;
        fprintf ('                    case GB_%s_opcode :\n', upper (ad)) ;
        fprintf ('\n') ;
        fprintf ('                        switch (zcode)\n') ;
        fprintf ('                        {\n') ;
        for c = nonbool
            n = n + 1 ;
            T = upper (c {1}) ;
            s = sprintf ('GB_%s_code', T) ;
            fprintf ('                            case %-14s: ', s) ;
            s = sprintf ('GxB_%s_%s_%s', ad, fmult, T) ;
            fprintf ('return (%-23s) ;\n', s) ;
        end
        fprintf ('                            default : ;\n') ;
        fprintf ('                        }\n') ;
        fprintf ('                        break ;\n') ;
    end
    fprintf ('\n') ;
    fprintf ('                    default : ;\n') ;
    fprintf ('                }\n') ;
end
fprintf ('\n') ;
fprintf ('            default : ;\n') ;
fprintf ('        }\n') ;
fprintf ('    }\n') ;

assert (n == 1000) ;

% 300: x,y nonboolean, z boolean: 6 * 4 * 10

fprintf ('    else if (xycode != GB_BOOL_code)\n') ;
fprintf ('    {\n') ;
fprintf ('        switch (mult_opcode)\n') ;
fprintf ('        {\n') ;
for mult = { 'eq', 'ne', 'gt', 'lt', 'ge', 'le' }
    fmult = upper (mult {1}) ;
    fprintf ('\n') ;
    fprintf ('            case GB_%s_opcode : // with (5 bool monoids) x (10 nonboolean types)\n\n', fmult) ;
    fprintf ('                switch (add_opcode)\n') ;
    fprintf ('                {\n') ;
    for add = { 'Lor', 'Land', 'Lxor', 'eq', 'any' }
        ad = upper (add {1}) ;
        fprintf ('\n') ;
        fprintf ('                    case GB_%s_opcode :\n', upper (ad)) ;
        fprintf ('\n') ;
        fprintf ('                        switch (xycode)\n') ;
        fprintf ('                        {\n') ;
        for c = nonbool
            n = n + 1 ;
            T = upper (c {1}) ;
            s = sprintf ('GB_%s_code', T) ;
            fprintf ('                            case %-14s: ', s) ;
            s = sprintf ('GxB_%s_%s_%s', ad, fmult, T) ;
            fprintf ('return (%-23s) ;\n', s) ;
        end
        fprintf ('                            default : ;\n') ;
        fprintf ('                        }\n') ;
        fprintf ('                        break ;\n') ;
    end
    fprintf ('\n') ;
    fprintf ('                    default : ;\n') ;
    fprintf ('                }\n') ;
end
fprintf ('\n') ;
fprintf ('            default : ;\n') ;
fprintf ('        }\n') ;
fprintf ('    }\n') ;

assert (n == 1300) ;

% 55: x,y,z all boolean: 11 * 5
fprintf ('    else // purely boolean semirings\n') ;

fprintf ('    {\n') ;
fprintf ('        switch (mult_opcode)\n') ;
fprintf ('        {\n') ;
for mult = { 'first', 'second', 'pair', 'Lor', 'Land', 'Lxor', ...
             'eq', 'gt', 'lt', 'ge', 'le' }
    fmult = upper (mult {1}) ;
    fprintf ('\n') ;
    fprintf ('            case GB_%s_opcode :\n\n', fmult) ;
    fprintf ('                switch (add_opcode)\n') ;
    fprintf ('                {\n') ;
    for add = { 'Lor', 'Land', 'Lxor', 'eq', 'any' }
        n = n + 1 ;
        ad = upper (add {1}) ;
        s = sprintf ('GB_%s_opcode', ad) ;
        fprintf ('                    case %-20s : ', s) ;
        s = sprintf ('GxB_%s_%s_BOOL', ad, fmult) ;
        fprintf ('return (%-23s) ;\n', s) ;
    end
    fprintf ('                    default : ;\n') ;
    fprintf ('                }\n') ;
end
fprintf ('\n') ;
fprintf ('            default : ;\n') ;
fprintf ('        }\n') ;
fprintf ('    }\n') ;

fprintf ('semirings: %d\n', n) ;

