function [fname, unsigned, bits] = codegen_type (xytype)
%CODEGEN_TYPE determine C type name, signed, and # bits a type
unsigned = (xytype (1) == 'u') ;
switch (xytype)
    case 'bool'
        fname = 'bool' ;
        bits = 1 ;
    case 'int8_t'
        fname = 'int8' ;
        bits = 8 ;
    case 'uint8_t'
        fname = 'uint8' ;
        bits = 8 ;
    case 'int16_t'
        fname = 'int16' ;
        bits = 16 ;
    case 'uint16_t'
        fname = 'uint16' ;
        bits = 16 ;
    case 'int32_t'
        fname = 'int32' ;
        bits = 32 ;
    case 'uint32_t'
        fname = 'uint32' ;
        bits = 32 ;
    case 'int64_t'
        fname = 'int64' ;
        bits = 64 ;
    case 'uint64_t'
        fname = 'uint64' ;
        bits = 64 ;
    case 'float'
        fname = 'fp32' ;
        bits = 32 ;
    case 'double'
        fname = 'fp64' ;
        bits = 64 ;
    case 'GB_void'
        fname = 'any' ;
        bits = nan ;
end

