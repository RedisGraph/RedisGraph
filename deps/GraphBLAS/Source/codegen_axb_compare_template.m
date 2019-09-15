function codegen_axb_compare_template (multop, bmult, mult)
%CODEGEN_AXB_COMPARE_TEMPLATE create a function for a semiring with a TxT->bool multiplier

fprintf ('\n%-7s', multop) ;

% lor monoid
add = 'w = (w || t)' ;
if (~isempty (bmult))
codegen_axb_method ('lor', multop, add, bmult, 'bool', 'bool'    , 'false', 'true') ;
end
codegen_axb_method ('lor', multop, add,  mult, 'bool', 'int8_t'  , 'false', 'true') ;
codegen_axb_method ('lor', multop, add,  mult, 'bool', 'uint8_t' , 'false', 'true') ;
codegen_axb_method ('lor', multop, add,  mult, 'bool', 'int16_t' , 'false', 'true') ;
codegen_axb_method ('lor', multop, add,  mult, 'bool', 'uint16_t', 'false', 'true') ;
codegen_axb_method ('lor', multop, add,  mult, 'bool', 'int32_t' , 'false', 'true') ;
codegen_axb_method ('lor', multop, add,  mult, 'bool', 'uint32_t', 'false', 'true') ;
codegen_axb_method ('lor', multop, add,  mult, 'bool', 'int64_t' , 'false', 'true') ;
codegen_axb_method ('lor', multop, add,  mult, 'bool', 'uint64_t', 'false', 'true') ;
codegen_axb_method ('lor', multop, add,  mult, 'bool', 'float'   , 'false', 'true') ;
codegen_axb_method ('lor', multop, add,  mult, 'bool', 'double'  , 'false', 'true') ;

% land monoid
add = 'w = (w && t)' ;
if (~isempty (bmult))
codegen_axb_method ('land', multop, add, bmult, 'bool', 'bool'    , 'true', 'false') ;
end
codegen_axb_method ('land', multop, add,  mult, 'bool', 'int8_t'  , 'true', 'false') ;
codegen_axb_method ('land', multop, add,  mult, 'bool', 'uint8_t' , 'true', 'false') ;
codegen_axb_method ('land', multop, add,  mult, 'bool', 'int16_t' , 'true', 'false') ;
codegen_axb_method ('land', multop, add,  mult, 'bool', 'uint16_t', 'true', 'false') ;
codegen_axb_method ('land', multop, add,  mult, 'bool', 'int32_t' , 'true', 'false') ;
codegen_axb_method ('land', multop, add,  mult, 'bool', 'uint32_t', 'true', 'false') ;
codegen_axb_method ('land', multop, add,  mult, 'bool', 'int64_t' , 'true', 'false') ;
codegen_axb_method ('land', multop, add,  mult, 'bool', 'uint64_t', 'true', 'false') ;
codegen_axb_method ('land', multop, add,  mult, 'bool', 'float'   , 'true', 'false') ;
codegen_axb_method ('land', multop, add,  mult, 'bool', 'double'  , 'true', 'false') ;

% lxor monoid
add = 'w = (w != t)' ;
if (~isempty (bmult))
codegen_axb_method ('lxor', multop, add, bmult, 'bool', 'bool'    , 'false', [ ]) ;
end
codegen_axb_method ('lxor', multop, add,  mult, 'bool', 'int8_t'  , 'false', [ ]) ;
codegen_axb_method ('lxor', multop, add,  mult, 'bool', 'uint8_t' , 'false', [ ]) ;
codegen_axb_method ('lxor', multop, add,  mult, 'bool', 'int16_t' , 'false', [ ]) ;
codegen_axb_method ('lxor', multop, add,  mult, 'bool', 'uint16_t', 'false', [ ]) ;
codegen_axb_method ('lxor', multop, add,  mult, 'bool', 'int32_t' , 'false', [ ]) ;
codegen_axb_method ('lxor', multop, add,  mult, 'bool', 'uint32_t', 'false', [ ]) ;
codegen_axb_method ('lxor', multop, add,  mult, 'bool', 'int64_t' , 'false', [ ]) ;
codegen_axb_method ('lxor', multop, add,  mult, 'bool', 'uint64_t', 'false', [ ]) ;
codegen_axb_method ('lxor', multop, add,  mult, 'bool', 'float'   , 'false', [ ]) ;
codegen_axb_method ('lxor', multop, add,  mult, 'bool', 'double'  , 'false', [ ]) ;

% eq monoid
add = 'w = (w == t)' ;
if (~isempty (bmult))
codegen_axb_method ('eq', multop, add, bmult, 'bool', 'bool'    , 'true', [ ]) ;
end
codegen_axb_method ('eq', multop, add,  mult, 'bool', 'int8_t'  , 'true', [ ]) ;
codegen_axb_method ('eq', multop, add,  mult, 'bool', 'uint8_t' , 'true', [ ]) ;
codegen_axb_method ('eq', multop, add,  mult, 'bool', 'int16_t' , 'true', [ ]) ;
codegen_axb_method ('eq', multop, add,  mult, 'bool', 'uint16_t', 'true', [ ]) ;
codegen_axb_method ('eq', multop, add,  mult, 'bool', 'int32_t' , 'true', [ ]) ;
codegen_axb_method ('eq', multop, add,  mult, 'bool', 'uint32_t', 'true', [ ]) ;
codegen_axb_method ('eq', multop, add,  mult, 'bool', 'int64_t' , 'true', [ ]) ;
codegen_axb_method ('eq', multop, add,  mult, 'bool', 'uint64_t', 'true', [ ]) ;
codegen_axb_method ('eq', multop, add,  mult, 'bool', 'float'   , 'true', [ ]) ;
codegen_axb_method ('eq', multop, add,  mult, 'bool', 'double'  , 'true', [ ]) ;


