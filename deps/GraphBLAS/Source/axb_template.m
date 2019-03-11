function axb_template (multop, do_boolean, imult, fmult, dmult, handle_flipxy)
%AXB_TEMPLATE create a function for a semiring with a TxT->T multiplier

if (nargin < 4)
    fmult = [ ] ;
end

if (nargin < 5)
    dmult = [ ] ;
end

if (isempty (fmult))
    fmult = imult ;
end

if (isempty (dmult))
    dmult = fmult ;
end

if (nargin < 6)
    handle_flipxy = 0 ;
end

% min monoid: all are terminal
add = 'w = GB_IMIN (w,t)' ;
axb_method ('min', multop, add, imult, 'int8_t'  , 'int8_t'  , 'INT8_MAX'  , 'INT8_MIN'  , handle_flipxy) ;
axb_method ('min', multop, add, imult, 'int16_t' , 'int16_t' , 'INT16_MAX' , 'INT16_MIN' , handle_flipxy) ;
axb_method ('min', multop, add, imult, 'int32_t' , 'int32_t' , 'INT32_MAX' , 'INT32_MIN' , handle_flipxy) ;
axb_method ('min', multop, add, imult, 'int64_t' , 'int64_t' , 'INT64_MAX' , 'INT64_MIN' , handle_flipxy) ;
axb_method ('min', multop, add, imult, 'uint8_t' , 'uint8_t' , 'UINT8_MAX' , '0'         , handle_flipxy) ;
axb_method ('min', multop, add, imult, 'uint16_t', 'uint16_t', 'UINT16_MAX', '0'         , handle_flipxy) ;
axb_method ('min', multop, add, imult, 'uint32_t', 'uint32_t', 'UINT32_MAX', '0'         , handle_flipxy) ;
axb_method ('min', multop, add, imult, 'uint64_t', 'uint64_t', 'UINT64_MAX', '0'         , handle_flipxy) ;
add = 'w = fminf (w,t)' ;
axb_method ('min', multop, add, fmult, 'float'   , 'float'   , 'INFINITY'  , '-INFINITY' , handle_flipxy) ;
add = 'w = fmin (w,t)' ;
axb_method ('min', multop, add, dmult, 'double'  , 'double'  , 'INFINITY'  , '-INFINITY' , handle_flipxy) ;

% max monoid: all are terminal
add = 'w = GB_IMAX (w,t)' ;
axb_method ('max', multop, add, imult, 'int8_t'  , 'int8_t'  , 'INT8_MIN'  , 'INT8_MAX'  , handle_flipxy) ;
axb_method ('max', multop, add, imult, 'int16_t' , 'int16_t' , 'INT16_MIN' , 'INT16_MAX' , handle_flipxy) ;
axb_method ('max', multop, add, imult, 'int32_t' , 'int32_t' , 'INT32_MIN' , 'INT32_MAX' , handle_flipxy) ;
axb_method ('max', multop, add, imult, 'int64_t' , 'int64_t' , 'INT64_MIN' , 'INT64_MAX' , handle_flipxy) ;
axb_method ('max', multop, add, imult, 'uint8_t' , 'uint8_t' , '0'         , 'UINT8_MAX' , handle_flipxy) ;
axb_method ('max', multop, add, imult, 'uint16_t', 'uint16_t', '0'         , 'UINT16_MAX', handle_flipxy) ;
axb_method ('max', multop, add, imult, 'uint32_t', 'uint32_t', '0'         , 'UINT32_MAX', handle_flipxy) ;
axb_method ('max', multop, add, imult, 'uint64_t', 'uint64_t', '0'         , 'UINT64_MAX', handle_flipxy) ;
add = 'w = fmaxf (w,t)' ;
axb_method ('max', multop, add, fmult, 'float'   , 'float'   , '-INFINITY' , 'INFINITY'  , handle_flipxy) ;
add = 'w = fmax (w,t)' ;
axb_method ('max', multop, add, dmult, 'double'  , 'double'  , '-INFINITY' , 'INFINITY'  , handle_flipxy) ;

% plus monoid: none are terminal
add = 'w += t' ;
axb_method ('plus', multop, add, imult, 'int8_t'  , 'int8_t'  , '0', [ ], handle_flipxy) ;
axb_method ('plus', multop, add, imult, 'uint8_t' , 'uint8_t' , '0', [ ], handle_flipxy) ;
axb_method ('plus', multop, add, imult, 'int16_t' , 'int16_t' , '0', [ ], handle_flipxy) ;
axb_method ('plus', multop, add, imult, 'uint16_t', 'uint16_t', '0', [ ], handle_flipxy) ;
axb_method ('plus', multop, add, imult, 'int32_t' , 'int32_t' , '0', [ ], handle_flipxy) ;
axb_method ('plus', multop, add, imult, 'uint32_t', 'uint32_t', '0', [ ], handle_flipxy) ;
axb_method ('plus', multop, add, imult, 'int64_t' , 'int64_t' , '0', [ ], handle_flipxy) ;
axb_method ('plus', multop, add, imult, 'uint64_t', 'uint64_t', '0', [ ], handle_flipxy) ;
axb_method ('plus', multop, add, fmult, 'float'   , 'float'   , '0', [ ], handle_flipxy) ;
axb_method ('plus', multop, add, dmult, 'double'  , 'double'  , '0', [ ], handle_flipxy) ;

% times monoid: integers are terminal, float and double are not
add = 'w *= t' ;
axb_method ('times', multop, add, imult, 'int8_t'  , 'int8_t'  , '1', '0', handle_flipxy) ;
axb_method ('times', multop, add, imult, 'uint8_t' , 'uint8_t' , '1', '0', handle_flipxy) ;
axb_method ('times', multop, add, imult, 'int16_t' , 'int16_t' , '1', '0', handle_flipxy) ;
axb_method ('times', multop, add, imult, 'uint16_t', 'uint16_t', '1', '0', handle_flipxy) ;
axb_method ('times', multop, add, imult, 'int32_t' , 'int32_t' , '1', '0', handle_flipxy) ;
axb_method ('times', multop, add, imult, 'uint32_t', 'uint32_t', '1', '0', handle_flipxy) ;
axb_method ('times', multop, add, imult, 'int64_t' , 'int64_t' , '1', '0', handle_flipxy) ;
axb_method ('times', multop, add, imult, 'uint64_t', 'uint64_t', '1', '0', handle_flipxy) ;
axb_method ('times', multop, add, fmult, 'float'   , 'float'   , '1', [ ], handle_flipxy) ;
axb_method ('times', multop, add, dmult, 'double'  , 'double'  , '1', [ ], handle_flipxy) ;

% boolean monoids: lor, land are terminal; lxor, eq are not
if (do_boolean)
    axb_method ('lor',  multop, 'w = (w || t)', imult, 'bool', 'bool', 'false', 'true' , handle_flipxy) ;
    axb_method ('land', multop, 'w = (w && t)', imult, 'bool', 'bool', 'true' , 'false', handle_flipxy) ;
    axb_method ('lxor', multop, 'w = (w != t)', imult, 'bool', 'bool', 'false', [ ]    , handle_flipxy) ;
    axb_method ('eq',   multop, 'w = (w == t)', imult, 'bool', 'bool', 'true' , [ ]    , handle_flipxy) ;
end


