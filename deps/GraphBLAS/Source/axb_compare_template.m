function axb_compare_template (multop, do_boolean, imult, fmult)
%AXB_COMPARE_TEMPLATE create a function for a semiring with a TxT->bool multiplier

if (nargin < 4)
    fmult = imult ;
end

% lor monoid
add = 'w = (w || t)' ;
if (do_boolean)
axb_method ('lor', multop, add, imult, 'bool', 'bool'    , 'false') ;
end
axb_method ('lor', multop, add, imult, 'bool', 'int8_t'  , 'false') ;
axb_method ('lor', multop, add, imult, 'bool', 'uint8_t' , 'false') ;
axb_method ('lor', multop, add, imult, 'bool', 'int16_t' , 'false') ;
axb_method ('lor', multop, add, imult, 'bool', 'uint16_t', 'false') ;
axb_method ('lor', multop, add, imult, 'bool', 'int32_t' , 'false') ;
axb_method ('lor', multop, add, imult, 'bool', 'uint32_t', 'false') ;
axb_method ('lor', multop, add, imult, 'bool', 'int64_t' , 'false') ;
axb_method ('lor', multop, add, imult, 'bool', 'uint64_t', 'false') ;
axb_method ('lor', multop, add, fmult, 'bool', 'float'   , 'false') ;
axb_method ('lor', multop, add, fmult, 'bool', 'double'  , 'false') ;

% land monoid
add = 'w = (w && t)' ;
if (do_boolean)
axb_method ('land', multop, add, imult, 'bool', 'bool'    , 'true') ;
end
axb_method ('land', multop, add, imult, 'bool', 'int8_t'  , 'true') ;
axb_method ('land', multop, add, imult, 'bool', 'uint8_t' , 'true') ;
axb_method ('land', multop, add, imult, 'bool', 'int16_t' , 'true') ;
axb_method ('land', multop, add, imult, 'bool', 'uint16_t', 'true') ;
axb_method ('land', multop, add, imult, 'bool', 'int32_t' , 'true') ;
axb_method ('land', multop, add, imult, 'bool', 'uint32_t', 'true') ;
axb_method ('land', multop, add, imult, 'bool', 'int64_t' , 'true') ;
axb_method ('land', multop, add, imult, 'bool', 'uint64_t', 'true') ;
axb_method ('land', multop, add, fmult, 'bool', 'float'   , 'true') ;
axb_method ('land', multop, add, fmult, 'bool', 'double'  , 'true') ;

% lxor monoid
add = 'w = (w != t)' ;
if (do_boolean)
axb_method ('lxor', multop, add, imult, 'bool', 'bool'    , 'false') ;
end
axb_method ('lxor', multop, add, imult, 'bool', 'int8_t'  , 'false') ;
axb_method ('lxor', multop, add, imult, 'bool', 'uint8_t' , 'false') ;
axb_method ('lxor', multop, add, imult, 'bool', 'int16_t' , 'false') ;
axb_method ('lxor', multop, add, imult, 'bool', 'uint16_t', 'false') ;
axb_method ('lxor', multop, add, imult, 'bool', 'int32_t' , 'false') ;
axb_method ('lxor', multop, add, imult, 'bool', 'uint32_t', 'false') ;
axb_method ('lxor', multop, add, imult, 'bool', 'int64_t' , 'false') ;
axb_method ('lxor', multop, add, imult, 'bool', 'uint64_t', 'false') ;
axb_method ('lxor', multop, add, fmult, 'bool', 'float'   , 'false') ;
axb_method ('lxor', multop, add, fmult, 'bool', 'double'  , 'false') ;

% eq monoid
add = 'w = (w == t)' ;
if (do_boolean)
axb_method ('eq', multop, add, imult, 'bool', 'bool'    , 'true') ;
end
axb_method ('eq', multop, add, imult, 'bool', 'int8_t'  , 'true') ;
axb_method ('eq', multop, add, imult, 'bool', 'uint8_t' , 'true') ;
axb_method ('eq', multop, add, imult, 'bool', 'int16_t' , 'true') ;
axb_method ('eq', multop, add, imult, 'bool', 'uint16_t', 'true') ;
axb_method ('eq', multop, add, imult, 'bool', 'int32_t' , 'true') ;
axb_method ('eq', multop, add, imult, 'bool', 'uint32_t', 'true') ;
axb_method ('eq', multop, add, imult, 'bool', 'int64_t' , 'true') ;
axb_method ('eq', multop, add, imult, 'bool', 'uint64_t', 'true') ;
axb_method ('eq', multop, add, fmult, 'bool', 'float'   , 'true') ;
axb_method ('eq', multop, add, fmult, 'bool', 'double'  , 'true') ;


