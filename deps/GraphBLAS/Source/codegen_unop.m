function codegen_unop
%CODEGEN_UNOP create functions for all unary operators
%
% This function creates all files of the form GB_unop__*.[ch],
% and the include file GB_unop__include.h.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

fprintf ('\nunary operators:\n') ;

f = fopen ('Generated/GB_unop__include.h', 'w') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '// GB_unop__include.h: definitions for GB_unop__*.c\n') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '\n') ;
fprintf (f, '// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.\n') ;
fprintf (f, '// SPDX-License-Identifier: Apache-2.0\n\n') ;
fprintf (f, '// This file has been automatically generated from Generator/GB_unop.h') ;
fprintf (f, '\n\n') ;
fclose (f) ;

codegen_unop_template ('one', ...
    'true',                     ... % bool
    '1',                        ... % int
    '1',                        ... % uint
    '1',                        ... % float
    '1',                        ... % double
    'GxB_CMPLXF(1,0)',          ... % GxB_FC32_t
    'GxB_CMPLX(1,0)') ;         ... % GxB_FC64_t

codegen_unop_template ('identity', ...
    'xarg',                     ... % bool
    'xarg',                     ... % int
    'xarg',                     ... % uint
    'xarg',                     ... % float
    'xarg',                     ... % double
    'xarg',                     ... % GxB_FC32_t
    'xarg') ;                   ... % GxB_FC64_t

codegen_unop_template ('ainv', ...
    'xarg',                     ... % bool
    '-xarg',                    ... % int
    '-xarg',                    ... % uint
    '-xarg',                    ... % float
    '-xarg',                    ... % double
    'GB_FC32_ainv (xarg)',      ... % GxB_FC32_t
    'GB_FC64_ainv (xarg)') ;    ... % GxB_FC64_t

codegen_unop_template ('abs', ...
    'xarg',                     ... % bool
    'GB_IABS (xarg)',           ... % int
    'xarg',                     ... % uint
    'fabsf (xarg)',             ... % float
    'fabs (xarg)',              ... % double
    [ ],                        ... % GxB_FC32_t (see below)
    [ ]) ;                      ... % GxB_FC64_t (see below)

codegen_unop_template ('minv', ...
    'true',                     ... % bool
    'GB_IMINV (xarg)',          ... % int
    'GB_IMINV (xarg)',          ... % uint
    '(1.0F)/xarg',              ... % float
    '1./xarg',                  ... % double
    'GB_FC32_minv (xarg)',      ... % GxB_FC32_t
    'GB_FC64_minv (xarg)') ;    ... % GxB_FC64_t

codegen_unop_template ('lnot',  ...
    '!xarg',                    ... % bool
    '!(xarg != 0)',             ... % int
    '!(xarg != 0)',             ... % uint
    '!(xarg != 0)',             ... % float
    '!(xarg != 0)',             ... % double
    [ ],                        ... % GxB_FC32_t
    [ ]) ;                      ... % GxB_FC64_t

codegen_unop_template ('bnot',  ...
    [ ],                        ... % bool
    '~(xarg)',                  ... % int
    '~(xarg)',                  ... % uint
    [ ],                        ... % float
    [ ],                        ... % double
    [ ],                        ... % GxB_FC32_t
    [ ]) ;                      ... % GxB_FC64_t

codegen_unop_template ('sqrt', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'sqrtf (xarg)',             ... % float
    'sqrt (xarg)',              ... % double
    'csqrtf (xarg)',            ... % GxB_FC32_t
    'csqrt (xarg)') ;           ... % GxB_FC64_t

codegen_unop_template ('log', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'logf (xarg)',              ... % float
    'log (xarg)',               ... % double
    'clogf (xarg)',             ... % GxB_FC32_t
    'clog (xarg)') ;            ... % GxB_FC64_t

codegen_unop_template ('exp', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'expf (xarg)',              ... % float
    'exp (xarg)',               ... % double
    'cexpf (xarg)',             ... % GxB_FC32_t
    'cexp (xarg)') ;            ... % GxB_FC64_t

codegen_unop_template ('sin', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'sinf (xarg)',              ... % float
    'sin (xarg)',               ... % double
    'csinf (xarg)',             ... % GxB_FC32_t
    'csin (xarg)') ;            ... % GxB_FC64_t

codegen_unop_template ('cos', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'cosf (xarg)',              ... % float
    'cos (xarg)',               ... % double
    'ccosf (xarg)',             ... % GxB_FC32_t
    'ccos (xarg)') ;            ... % GxB_FC64_t

codegen_unop_template ('tan', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'tanf (xarg)',              ... % float
    'tan (xarg)',               ... % double
    'ctanf (xarg)',             ... % GxB_FC32_t
    'ctan (xarg)') ;            ... % GxB_FC64_t

codegen_unop_template ('asin', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'asinf (xarg)',             ... % float
    'asin (xarg)',              ... % double
    'casinf (xarg)',            ... % GxB_FC32_t
    'casin (xarg)') ;           ... % GxB_FC64_t

codegen_unop_template ('acos', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'acosf (xarg)',             ... % float
    'acos (xarg)',              ... % double
    'cacosf (xarg)',            ... % GxB_FC32_t
    'cacos (xarg)') ;           ... % GxB_FC64_t

codegen_unop_template ('atan', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'atanf (xarg)',             ... % float
    'atan (xarg)',              ... % double
    'catanf (xarg)',            ... % GxB_FC32_t
    'catan (xarg)') ;           ... % GxB_FC64_t


codegen_unop_template ('sinh', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'sinhf (xarg)',             ... % float
    'sinh (xarg)',              ... % double
    'csinhf (xarg)',            ... % GxB_FC32_t
    'csinh (xarg)') ;           ... % GxB_FC64_t

codegen_unop_template ('cosh', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'coshf (xarg)',             ... % float
    'cosh (xarg)',              ... % double
    'ccoshf (xarg)',            ... % GxB_FC32_t
    'ccosh (xarg)') ;           ... % GxB_FC64_t

codegen_unop_template ('tanh', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'tanhf (xarg)',             ... % float
    'tanh (xarg)',              ... % double
    'ctanhf (xarg)',            ... % GxB_FC32_t
    'ctanh (xarg)') ;           ... % GxB_FC64_t

codegen_unop_template ('asinh', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'asinhf (xarg)',            ... % float
    'asinh (xarg)',             ... % double
    'casinhf (xarg)',           ... % GxB_FC32_t
    'casinh (xarg)') ;          ... % GxB_FC64_t

codegen_unop_template ('acosh', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'acoshf (xarg)',            ... % float
    'acosh (xarg)',             ... % double
    'cacoshf (xarg)',           ... % GxB_FC32_t
    'cacosh (xarg)') ;          ... % GxB_FC64_t

codegen_unop_template ('atanh', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'atanhf (xarg)',            ... % float
    'atanh (xarg)',             ... % double
    'catanhf (xarg)',           ... % GxB_FC32_t
    'catanh (xarg)') ;          ... % GxB_FC64_t

codegen_unop_template ('signum', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'GB_signumf (xarg)',        ... % float
    'GB_signum (xarg)',         ... % double
    'GB_csignumf (xarg)',       ... % GxB_FC32_t
    'GB_csignum (xarg)') ;      ... % GxB_FC64_t

codegen_unop_template ('ceil', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'ceilf (xarg)',             ... % float
    'ceil (xarg)',              ... % double
    'GB_cceilf (xarg)',         ... % GxB_FC32_t
    'GB_cceil (xarg)') ;        ... % GxB_FC64_t

codegen_unop_template ('floor', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'floorf (xarg)',            ... % float
    'floor (xarg)',             ... % double
    'GB_cfloorf (xarg)',        ... % GxB_FC32_t
    'GB_cfloor (xarg)') ;       ... % GxB_FC64_t

codegen_unop_template ('round', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'roundf (xarg)',            ... % float
    'round (xarg)',             ... % double
    'GB_croundf (xarg)',        ... % GxB_FC32_t
    'GB_cround (xarg)') ;       ... % GxB_FC64_t

codegen_unop_template ('trunc', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'truncf (xarg)',            ... % float
    'trunc (xarg)',             ... % double
    'GB_ctruncf (xarg)',        ... % GxB_FC32_t
    'GB_ctrunc (xarg)') ;       ... % GxB_FC64_t

codegen_unop_template ('exp2', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'exp2f (xarg)',             ... % float
    'exp2 (xarg)',              ... % double
    'GB_cexp2f (xarg)',         ... % GxB_FC32_t
    'GB_cexp2 (xarg)') ;        ... % GxB_FC64_t

codegen_unop_template ('expm1', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'expm1f (xarg)',            ... % float
    'expm1 (xarg)',             ... % double
    'GB_cexpm1f (xarg)',        ... % GxB_FC32_t
    'GB_cexpm1 (xarg)') ;       ... % GxB_FC64_t

codegen_unop_template ('log10', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'log10f (xarg)',            ... % float
    'log10 (xarg)',             ... % double
    'GB_clog10f (xarg)',        ... % GxB_FC32_t
    'GB_clog10 (xarg)') ;       ... % GxB_FC64_t

codegen_unop_template ('log1p', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'log1pf (xarg)',            ... % float
    'log1p (xarg)',             ... % double
    'GB_clog1pf (xarg)',        ... % GxB_FC32_t
    'GB_clog1p (xarg)') ;       ... % GxB_FC64_t

codegen_unop_template ('log2', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'log2f (xarg)',             ... % float
    'log2 (xarg)',              ... % double
    'GB_clog2f (xarg)',         ... % GxB_FC32_t
    'GB_clog2 (xarg)') ;        ... % GxB_FC64_t

codegen_unop_template ('frexpx', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'GB_frexpxf (xarg)',        ... % float
    'GB_frexpx (xarg)',         ... % double
    [ ],                        ... % GxB_FC32_t
    [ ]) ;                      ... % GxB_FC64_t

codegen_unop_template ('frexpe', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'GB_frexpef (xarg)',        ... % float
    'GB_frexpe (xarg)',         ... % double
    [ ],                        ... % GxB_FC32_t
    [ ]) ;                      ... % GxB_FC64_t

codegen_unop_template ('lgamma', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'lgammaf (xarg)',           ... % float
    'lgamma (xarg)',            ... % double
    [ ],                        ... % GxB_FC32_t
    [ ]) ;                      ... % GxB_FC64_t

codegen_unop_template ('tgamma', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'tgammaf (xarg)',           ... % float
    'tgamma (xarg)',            ... % double
    [ ],                        ... % GxB_FC32_t
    [ ]) ;                      ... % GxB_FC64_t

codegen_unop_template ('erf', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'erff (xarg)',              ... % float
    'erf (xarg)',               ... % double
    [ ],                        ... % GxB_FC32_t
    [ ]) ;                      ... % GxB_FC64_t

codegen_unop_template ('erfc', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    'erfcf (xarg)',             ... % float
    'erfc (xarg)',              ... % double
    [ ],                        ... % GxB_FC32_t
    [ ]) ;                      ... % GxB_FC64_t

codegen_unop_template ('conj', ...
    [ ],                        ... % bool
    [ ],                        ... % int
    [ ],                        ... % uint
    [ ],                        ... % float
    [ ],                        ... % double
    'conjf (xarg)',             ... % GxB_FC32_t
    'conj (xarg)') ;            ... % GxB_FC64_t

%-------------------------------------------------------------------------------
% z = f(x) where the type of z and x differ
%-------------------------------------------------------------------------------

% z = abs (x): x is complex, z is real
fprintf ('\nabs      ') ;
codegen_unop_method ('abs', 'cabsf (xarg)', 'GxB_FC32_t zarg = (xarg)', 'float' , 'GxB_FC32_t') ;
codegen_unop_method ('abs', 'cabs (xarg)' , 'GxB_FC64_t zarg = (xarg)', 'double', 'GxB_FC64_t') ;

% z = creal (x): x is complex, z is real
fprintf ('\ncreal    ') ;
codegen_unop_method ('creal', 'crealf (xarg)', 'GxB_FC32_t zarg = (xarg)', 'float' , 'GxB_FC32_t') ;
codegen_unop_method ('creal', 'creal (xarg)' , 'GxB_FC64_t zarg = (xarg)', 'double', 'GxB_FC64_t') ;

% z = cimag (x): x is complex, z is real
fprintf ('\ncimag    ') ;
codegen_unop_method ('cimag', 'cimagf (xarg)', 'GxB_FC32_t zarg = (xarg)', 'float' , 'GxB_FC32_t') ;
codegen_unop_method ('cimag', 'cimag (xarg)' , 'GxB_FC64_t zarg = (xarg)', 'double', 'GxB_FC64_t') ;

% z = carg (x): x is complex, z is real
fprintf ('\ncarg     ') ;
codegen_unop_method ('carg', 'cargf (xarg)', 'GxB_FC32_t zarg = (xarg)', 'float' , 'GxB_FC32_t') ;
codegen_unop_method ('carg', 'carg (xarg)' , 'GxB_FC64_t zarg = (xarg)', 'double', 'GxB_FC64_t') ;

% z = isinf (x): x is floating-point, z is bool
fprintf ('\nisinf    ') ;
codegen_unop_method ('isinf', 'isinf (xarg)'     , 'float zarg = (xarg)'     , 'bool', 'float') ;
codegen_unop_method ('isinf', 'isinf (xarg)'     , 'double zarg = (xarg)'    , 'bool', 'double') ;
codegen_unop_method ('isinf', 'GB_cisinff (xarg)', 'GxB_FC32_t zarg = (xarg)', 'bool', 'GxB_FC32_t') ;
codegen_unop_method ('isinf', 'GB_cisinf (xarg)' , 'GxB_FC64_t zarg = (xarg)', 'bool', 'GxB_FC64_t') ;

% z = isnan (x): x is floating-point, z is bool
fprintf ('\nisnan    ') ;
codegen_unop_method ('isnan', 'isnan (xarg)'     , 'float zarg = (xarg)'     , 'bool', 'float') ;
codegen_unop_method ('isnan', 'isnan (xarg)'     , 'double zarg = (xarg)'    , 'bool', 'double') ;
codegen_unop_method ('isnan', 'GB_cisnanf (xarg)', 'GxB_FC32_t zarg = (xarg)', 'bool', 'GxB_FC32_t') ;
codegen_unop_method ('isnan', 'GB_cisnan (xarg)' , 'GxB_FC64_t zarg = (xarg)', 'bool', 'GxB_FC64_t') ;

% z = isfinite (x): x is floating-point, z is bool
fprintf ('\nisfinite ') ;
codegen_unop_method ('isfinite', 'isfinite (xarg)'     , 'float zarg = (xarg)'     , 'bool', 'float') ;
codegen_unop_method ('isfinite', 'isfinite (xarg)'     , 'double zarg = (xarg)'    , 'bool', 'double') ;
codegen_unop_method ('isfinite', 'GB_cisfinitef (xarg)', 'GxB_FC32_t zarg = (xarg)', 'bool', 'GxB_FC32_t') ;
codegen_unop_method ('isfinite', 'GB_cisfinite (xarg)' , 'GxB_FC64_t zarg = (xarg)', 'bool', 'GxB_FC64_t') ;
fprintf ('\n') ;

