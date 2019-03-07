function axb_compare_template (multop, do_boolean, mult)
%AXB_COMPARE_TEMPLATE create a function for a semiring with a TxT->bool multiplier

% lor monoid
add = 'w = (w || t)' ;
if (do_boolean)
axb_method ('lor', multop, add, mult, 'bool', 'bool'    , 'false', 'true', 0) ;
end
axb_method ('lor', multop, add, mult, 'bool', 'int8_t'  , 'false', 'true', 0) ;
axb_method ('lor', multop, add, mult, 'bool', 'uint8_t' , 'false', 'true', 0) ;
axb_method ('lor', multop, add, mult, 'bool', 'int16_t' , 'false', 'true', 0) ;
axb_method ('lor', multop, add, mult, 'bool', 'uint16_t', 'false', 'true', 0) ;
axb_method ('lor', multop, add, mult, 'bool', 'int32_t' , 'false', 'true', 0) ;
axb_method ('lor', multop, add, mult, 'bool', 'uint32_t', 'false', 'true', 0) ;
axb_method ('lor', multop, add, mult, 'bool', 'int64_t' , 'false', 'true', 0) ;
axb_method ('lor', multop, add, mult, 'bool', 'uint64_t', 'false', 'true', 0) ;
axb_method ('lor', multop, add, mult, 'bool', 'float'   , 'false', 'true', 0) ;
axb_method ('lor', multop, add, mult, 'bool', 'double'  , 'false', 'true', 0) ;

% land monoid
add = 'w = (w && t)' ;
if (do_boolean)
axb_method ('land', multop, add, mult, 'bool', 'bool'    , 'true', 'false', 0) ;
end
axb_method ('land', multop, add, mult, 'bool', 'int8_t'  , 'true', 'false', 0) ;
axb_method ('land', multop, add, mult, 'bool', 'uint8_t' , 'true', 'false', 0) ;
axb_method ('land', multop, add, mult, 'bool', 'int16_t' , 'true', 'false', 0) ;
axb_method ('land', multop, add, mult, 'bool', 'uint16_t', 'true', 'false', 0) ;
axb_method ('land', multop, add, mult, 'bool', 'int32_t' , 'true', 'false', 0) ;
axb_method ('land', multop, add, mult, 'bool', 'uint32_t', 'true', 'false', 0) ;
axb_method ('land', multop, add, mult, 'bool', 'int64_t' , 'true', 'false', 0) ;
axb_method ('land', multop, add, mult, 'bool', 'uint64_t', 'true', 'false', 0) ;
axb_method ('land', multop, add, mult, 'bool', 'float'   , 'true', 'false', 0) ;
axb_method ('land', multop, add, mult, 'bool', 'double'  , 'true', 'false', 0) ;

% lxor monoid
add = 'w = (w != t)' ;
if (do_boolean)
axb_method ('lxor', multop, add, mult, 'bool', 'bool'    , 'false', [ ], 0) ;
end
axb_method ('lxor', multop, add, mult, 'bool', 'int8_t'  , 'false', [ ], 0) ;
axb_method ('lxor', multop, add, mult, 'bool', 'uint8_t' , 'false', [ ], 0) ;
axb_method ('lxor', multop, add, mult, 'bool', 'int16_t' , 'false', [ ], 0) ;
axb_method ('lxor', multop, add, mult, 'bool', 'uint16_t', 'false', [ ], 0) ;
axb_method ('lxor', multop, add, mult, 'bool', 'int32_t' , 'false', [ ], 0) ;
axb_method ('lxor', multop, add, mult, 'bool', 'uint32_t', 'false', [ ], 0) ;
axb_method ('lxor', multop, add, mult, 'bool', 'int64_t' , 'false', [ ], 0) ;
axb_method ('lxor', multop, add, mult, 'bool', 'uint64_t', 'false', [ ], 0) ;
axb_method ('lxor', multop, add, mult, 'bool', 'float'   , 'false', [ ], 0) ;
axb_method ('lxor', multop, add, mult, 'bool', 'double'  , 'false', [ ], 0) ;

% eq monoid
add = 'w = (w == t)' ;
if (do_boolean)
axb_method ('eq', multop, add, mult, 'bool', 'bool'    , 'true', [ ], 0) ;
end
axb_method ('eq', multop, add, mult, 'bool', 'int8_t'  , 'true', [ ], 0) ;
axb_method ('eq', multop, add, mult, 'bool', 'uint8_t' , 'true', [ ], 0) ;
axb_method ('eq', multop, add, mult, 'bool', 'int16_t' , 'true', [ ], 0) ;
axb_method ('eq', multop, add, mult, 'bool', 'uint16_t', 'true', [ ], 0) ;
axb_method ('eq', multop, add, mult, 'bool', 'int32_t' , 'true', [ ], 0) ;
axb_method ('eq', multop, add, mult, 'bool', 'uint32_t', 'true', [ ], 0) ;
axb_method ('eq', multop, add, mult, 'bool', 'int64_t' , 'true', [ ], 0) ;
axb_method ('eq', multop, add, mult, 'bool', 'uint64_t', 'true', [ ], 0) ;
axb_method ('eq', multop, add, mult, 'bool', 'float'   , 'true', [ ], 0) ;
axb_method ('eq', multop, add, mult, 'bool', 'double'  , 'true', [ ], 0) ;

