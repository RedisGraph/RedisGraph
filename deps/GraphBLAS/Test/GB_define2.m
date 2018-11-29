function GBdefine
%GB_DEFINE2 construct part of the GB.h file, to allow user-defined objects
% defined at compile time to access built-in objects

types = {
'BOOL',
'INT8',
'UINT8',
'INT16',
'UINT16',
'INT32',
'UINT32',
'INT64',
'UINT64',
'FP32',
'FP64' } ;

ctypes = {
'bool',
'int8_t',
'uint8_t',
'int16_t',
'uint16_t',
'int32_t',
'uint32_t',
'int64_t',
'uint64_t',
'float',
'double' } ;

fprintf ('\n//------------------------------------------------------\n') ;
fprintf ('// built-in types\n') ;
fprintf ('//------------------------------------------------------\n\n') ;

    for t = 1:length (types)
        gt = types  {t} ;
        ct = ctypes {t} ;
        fprintf ('#define GB_DEF_GrB_%s_type %s\n', gt, ct) ;
    end

fprintf ('\n//------------------------------------------------------\n') ;
fprintf ('// built-in unary operators\n') ;
fprintf ('//------------------------------------------------------\n\n') ;

uops = {
'IDENTITY',
'AINV'
'MINV',
'LNOT',     % lnot_type and following are extensions
'ONE',
'ABS' } ;

for k = 1:length (uops)
    op = uops {k} ;
    fprintf ('// op: %s\n', op) ;
    for t = 1:length (types)
        gt = types  {t} ;
        ct = ctypes {t} ;
        if (k <= 3)
            kind = 'r' ;
        else
            kind = 'x' ;
        end
        fprintf ('#define GB_DEF_G%sB_%s_%s_function GB_%s_f_%s\n', ...
            kind, op, gt, op, gt) ;
        fprintf ('#define GB_DEF_G%sB_%s_%s_ztype %s\n', kind, op, gt, ct) ;
        fprintf ('#define GB_DEF_G%sB_%s_%s_xtype %s\n', kind, op, gt, ct) ;
        fprintf ('\n') ;
    end
end

% GrB_LNOT
fprintf ('#define GB_DEF_GrB_LNOT_function GB_LNOT_f_BOOL\n') ;
fprintf ('#define GB_DEF_GrB_LNOT_ztype bool\n') ;
fprintf ('#define GB_DEF_GrB_LNOT_xtype bool\n') ;

fprintf ('\n//------------------------------------------------------\n') ;
fprintf ('// binary operators of the form z=f(x,y): TxT -> T\n') ;
fprintf ('//------------------------------------------------------\n\n') ;

ops1 = {
'FIRST',
'SECOND',
'MIN',
'MAX',
'PLUS',
'MINUS',
'TIMES',
'DIV',
'ISEQ',     % iseq and following are extensions
'ISNE',
'ISGT',
'ISLT',
'ISGE',
'ISLE',
'LOR',
'LAND',
'LXOR' } ;

for k = 1:length (ops1)
    op = ops1 {k} ;
    fprintf ('// op: %s\n', op) ;
    for t = 1:length (types)
        gt = types  {t} ;
        ct = ctypes {t} ;
        if (k <= 8)
            kind = 'r' ;
        else
            kind = 'x' ;
        end
        fprintf ('#define GB_DEF_G%sB_%s_%s_function GB_%s_f_%s\n', ...
            kind, op, gt, op, gt) ;
        fprintf ('#define GB_DEF_G%sB_%s_%s_ztype %s\n', kind, op, gt, ct) ;
        fprintf ('#define GB_DEF_G%sB_%s_%s_xtype %s\n', kind, op, gt, ct) ;
        fprintf ('#define GB_DEF_G%sB_%s_%s_ytype %s\n', kind, op, gt, ct) ;
        fprintf ('\n') ;
    end
end

fprintf ('\n//------------------------------------------------------\n') ;
fprintf ('// binary operators of the form z=f(x,y): TxT -> bool\n') ;
fprintf ('//------------------------------------------------------\n\n') ;

ops2 = {
'EQ',
'NE',
'GT',
'LT',
'GE',
'LE' } ;

for k = 1:length (ops2)
    op = ops2 {k} ;
    fprintf ('// op: %s\n', op) ;
    for t = 1:length (types)
        gt = types  {t} ;
        ct = ctypes {t} ;
        fprintf ('#define GB_DEF_GrB_%s_%s_function GB_%s_f_%s\n', ...
            op, gt, op, gt) ;
        fprintf ('#define GB_DEF_GrB_%s_%s_ztype bool\n', op, gt) ;
        fprintf ('#define GB_DEF_GrB_%s_%s_xtype %s\n', op, gt, ct) ;
        fprintf ('#define GB_DEF_GrB_%s_%s_ytype %s\n', op, gt, ct) ;
        fprintf ('\n') ;
    end
end

fprintf ('\n//------------------------------------------------------\n') ;
fprintf ('// binary operators of the form z=f(x,y): bool x bool -> bool\n') ;
fprintf ('//------------------------------------------------------\n\n') ;

ops3 = {
'LOR',
'LAND',
'LXOR' } ;

for k = 1:length (ops3)
        op = ops3 {k} ;
        fprintf ('#define GB_DEF_GrB_%s_function GB_%s_f_BOOL\n', op, op) ;
        fprintf ('#define GB_DEF_GrB_%s_ztype bool\n', op) ;
        fprintf ('#define GB_DEF_GrB_%s_xtype bool\n', op) ;
        fprintf ('#define GB_DEF_GrB_%s_ytype bool\n', op) ;
        fprintf ('\n') ;
end


fprintf ('\n//------------------------------------------------------\n') ;
fprintf ('// built-in monoids\n') ;
fprintf ('//------------------------------------------------------\n\n') ;

mons = {
'MIN',
'MAX',
'PLUS',
'TIMES' } ;

for k = 1:length (mons)
    op = mons {k} ;
    fprintf ('// op: %s\n', op) ;
    for t = 1:length (types)
        gt = types  {t} ;
        ct = ctypes {t} ;
        fprintf ('#define GB_DEF_GxB_%s_%s_add GB_%s_f_%s\n', ...
            op, gt, op, gt) ;
    end
end

fprintf ('\n') ;
fprintf ('#define GB_DEF_GxB_MIN_INT8_MONOID_identity   INT8_MAX\n') ;
fprintf ('#define GB_DEF_GxB_MIN_UINT8_MONOID_identity  UINT8_MAX\n') ;
fprintf ('#define GB_DEF_GxB_MIN_INT16_MONOID_identity  INT16_MAX\n') ;
fprintf ('#define GB_DEF_GxB_MIN_UINT16_MONOID_identity UINT16_MAX\n') ;
fprintf ('#define GB_DEF_GxB_MIN_INT32_MONOID_identity  INT32_MAX\n') ;
fprintf ('#define GB_DEF_GxB_MIN_UINT32_MONOID_identity UINT32_MAX\n') ;
fprintf ('#define GB_DEF_GxB_MIN_INT64_MONOID_identity  INT64_MAX\n') ;
fprintf ('#define GB_DEF_GxB_MIN_UINT64_MONOID_identity UINT64_MAX\n') ;
fprintf ('#define GB_DEF_GxB_MIN_FP32_MONOID_identity   INFINITY\n') ;
fprintf ('#define GB_DEF_GxB_MIN_FP64_MONOID_identity   INFINITY\n') ;

fprintf ('\n') ;
fprintf ('#define GB_DEF_GxB_MAX_INT8_MONOID_identity   INT8_MIN\n') ;
fprintf ('#define GB_DEF_GxB_MAX_UINT8_MONOID_identity  0\n') ;
fprintf ('#define GB_DEF_GxB_MAX_INT16_MONOID_identity  INT16_MIN\n') ;
fprintf ('#define GB_DEF_GxB_MAX_UINT16_MONOID_identity 0\n') ;
fprintf ('#define GB_DEF_GxB_MAX_INT32_MONOID_identity  INT32_MIN\n') ;
fprintf ('#define GB_DEF_GxB_MAX_UINT32_MONOID_identity 0\n') ;
fprintf ('#define GB_DEF_GxB_MAX_INT64_MONOID_identity  INT64_MIN\n') ;
fprintf ('#define GB_DEF_GxB_MAX_UINT64_MONOID_identity 0\n') ;
fprintf ('#define GB_DEF_GxB_MAX_FP32_MONOID_identity   (-INFINITY)\n') ;
fprintf ('#define GB_DEF_GxB_MAX_FP64_MONOID_identity   (-INFINITY)\n') ;

fprintf ('\n') ;
fprintf ('#define GB_DEF_GxB_PLUS_INT8_MONOID_identity   0\n') ;
fprintf ('#define GB_DEF_GxB_PLUS_UINT8_MONOID_identity  0\n') ;
fprintf ('#define GB_DEF_GxB_PLUS_INT16_MONOID_identity  0\n') ;
fprintf ('#define GB_DEF_GxB_PLUS_UINT16_MONOID_identity 0\n') ;
fprintf ('#define GB_DEF_GxB_PLUS_INT32_MONOID_identity  0\n') ;
fprintf ('#define GB_DEF_GxB_PLUS_UINT32_MONOID_identity 0\n') ;
fprintf ('#define GB_DEF_GxB_PLUS_INT64_MONOID_identity  0\n') ;
fprintf ('#define GB_DEF_GxB_PLUS_UINT64_MONOID_identity 0\n') ;
fprintf ('#define GB_DEF_GxB_PLUS_FP32_MONOID_identity   0\n') ;
fprintf ('#define GB_DEF_GxB_PLUS_FP64_MONOID_identity   0\n') ;

fprintf ('\n') ;
fprintf ('#define GB_DEF_GxB_TIMES_INT8_MONOID_identity   1\n') ;
fprintf ('#define GB_DEF_GxB_TIMES_UINT8_MONOID_identity  1\n') ;
fprintf ('#define GB_DEF_GxB_TIMES_INT16_MONOID_identity  1\n') ;
fprintf ('#define GB_DEF_GxB_TIMES_UINT16_MONOID_identity 1\n') ;
fprintf ('#define GB_DEF_GxB_TIMES_INT32_MONOID_identity  1\n') ;
fprintf ('#define GB_DEF_GxB_TIMES_UINT32_MONOID_identity 1\n') ;
fprintf ('#define GB_DEF_GxB_TIMES_INT64_MONOID_identity  1\n') ;
fprintf ('#define GB_DEF_GxB_TIMES_UINT64_MONOID_identity 1\n') ;
fprintf ('#define GB_DEF_GxB_TIMES_FP32_MONOID_identity   1\n') ;
fprintf ('#define GB_DEF_GxB_TIMES_FP64_MONOID_identity   1\n') ;

fprintf ('\n') ;
fprintf ('#define GB_DEF_GxB_LOR_BOOL_MONOID_identity    false\n') ;
fprintf ('#define GB_DEF_GxB_LAND_BOOL_MONOID_identity   true\n') ;
fprintf ('#define GB_DEF_GxB_LXOR_BOOL_MONOID_identity   false\n') ;
fprintf ('#define GB_DEF_GxB_EQ_BOOL_MONOID_identity     true\n') ;

% no need for built-in semirings


