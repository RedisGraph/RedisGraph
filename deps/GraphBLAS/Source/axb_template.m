function axb_template (multop, do_boolean, imult, fmult)

if (nargin < 4)
    fmult = imult ;
end

% min monoid
add = 'w = IMIN (w,t)' ;
axb_method ('min', multop, add, imult, 'int8_t'  , 'int8_t'  , 'INT8_MAX') ;
axb_method ('min', multop, add, imult, 'uint8_t' , 'uint8_t' , 'UINT8_MAX') ;
axb_method ('min', multop, add, imult, 'int16_t' , 'int16_t' , 'INT16_MAX') ;
axb_method ('min', multop, add, imult, 'uint16_t', 'uint16_t', 'UINT16_MAX') ;
axb_method ('min', multop, add, imult, 'int32_t' , 'int32_t' , 'INT32_MAX') ;
axb_method ('min', multop, add, imult, 'uint32_t', 'uint32_t', 'UINT32_MAX') ;
axb_method ('min', multop, add, imult, 'int64_t' , 'int64_t' , 'INT64_MAX') ;
axb_method ('min', multop, add, imult, 'uint64_t', 'uint64_t', 'UINT64_MAX') ;
add = 'w = FMIN (w,t)' ;
axb_method ('min', multop, add, fmult, 'float'   , 'float'   , 'INFINITY') ;
axb_method ('min', multop, add, fmult, 'double'  , 'double'  , 'INFINITY') ;

% max monoid
add = 'w = IMAX (w,t)' ;
axb_method ('max', multop, add, imult, 'int8_t'  , 'int8_t'  , 'INT8_MIN') ;
axb_method ('max', multop, add, imult, 'uint8_t' , 'uint8_t' , '0') ;
axb_method ('max', multop, add, imult, 'int16_t' , 'int16_t' , 'INT16_MIN') ;
axb_method ('max', multop, add, imult, 'uint16_t', 'uint16_t', '0') ;
axb_method ('max', multop, add, imult, 'int32_t' , 'int32_t' , 'INT32_MIN') ;
axb_method ('max', multop, add, imult, 'uint32_t', 'uint32_t', '0') ;
axb_method ('max', multop, add, imult, 'int64_t' , 'int64_t' , 'INT64_MIN') ;
axb_method ('max', multop, add, imult, 'uint64_t', 'uint64_t', '0') ;
add = 'w = FMAX (w,t)' ;
axb_method ('max', multop, add, fmult, 'float'   , 'float'   , '-INFINITY') ;
axb_method ('max', multop, add, fmult, 'double'  , 'double'  , '-INFINITY') ;

% plus monoid
add = 'w += t' ;
axb_method ('plus', multop, add, imult, 'int8_t'  , 'int8_t'  , '0') ;
axb_method ('plus', multop, add, imult, 'uint8_t' , 'uint8_t' , '0') ;
axb_method ('plus', multop, add, imult, 'int16_t' , 'int16_t' , '0') ;
axb_method ('plus', multop, add, imult, 'uint16_t', 'uint16_t', '0') ;
axb_method ('plus', multop, add, imult, 'int32_t' , 'int32_t' , '0') ;
axb_method ('plus', multop, add, imult, 'uint32_t', 'uint32_t', '0') ;
axb_method ('plus', multop, add, imult, 'int64_t' , 'int64_t' , '0') ;
axb_method ('plus', multop, add, imult, 'uint64_t', 'uint64_t', '0') ;
axb_method ('plus', multop, add, fmult, 'float'   , 'float'   , '0') ;
axb_method ('plus', multop, add, fmult, 'double'  , 'double'  , '0') ;

% times monoid
add = 'w *= t' ;
axb_method ('times', multop, add, imult, 'int8_t'  , 'int8_t'  , '1') ;
axb_method ('times', multop, add, imult, 'uint8_t' , 'uint8_t' , '1') ;
axb_method ('times', multop, add, imult, 'int16_t' , 'int16_t' , '1') ;
axb_method ('times', multop, add, imult, 'uint16_t', 'uint16_t', '1') ;
axb_method ('times', multop, add, imult, 'int32_t' , 'int32_t' , '1') ;
axb_method ('times', multop, add, imult, 'uint32_t', 'uint32_t', '1') ;
axb_method ('times', multop, add, imult, 'int64_t' , 'int64_t' , '1') ;
axb_method ('times', multop, add, imult, 'uint64_t', 'uint64_t', '1') ;
axb_method ('times', multop, add, fmult, 'float'   , 'float'   , '1') ;
axb_method ('times', multop, add, fmult, 'double'  , 'double'  , '1') ;

% boolean monoids
if (do_boolean)
    axb_method ('lor',  multop, 'w = (w || t)', imult, 'bool', 'bool', 'false');
    axb_method ('land', multop, 'w = (w && t)', imult, 'bool', 'bool', 'true') ;
    axb_method ('lxor', multop, 'w = (w != t)', imult, 'bool', 'bool', 'false');
    axb_method ('eq',   multop, 'w = (w == t)', imult, 'bool', 'bool', 'true') ;
end

