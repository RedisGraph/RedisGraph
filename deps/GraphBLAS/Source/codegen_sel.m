function codegen_sel
%CODEGEN_SEL create functions for all selection operators
%
% This function creates all files of the form GB_sel__*.c,
% and the include file GB_sel__include.h.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

fprintf ('\nselection operators:\n') ;

f = fopen ('Generated1/GB_sel__include.h', 'w') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '// GB_sel__include.h: definitions for GB_sel__*.c\n') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '\n') ;
fprintf (f, '// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.\n') ;
fprintf (f, '// SPDX-License-Identifier: Apache-2.0\n\n') ;
fprintf (f, '// This file has been automatically generated from Generator/GB_sel.h') ;
fprintf (f, '\n\n') ;
fclose (f) ;

%===============================================================================
% select ops where A is either iso or non-iso
%===============================================================================

% USER selectop:
fprintf ('\nselop      ') ;

% user select op, A is not iso
test_value = 'bool keep = op->selop_function (flipij ? j : i, flipij ? i : j, Ax +(p)*asize, ythunk)' ;
codegen_sel_method ('user', test_value, 'GB_void') ;

% user select op, A is iso
test_value = 'bool keep = op->selop_function (flipij ? j : i, flipij ? i : j, Ax, ythunk)' ;
codegen_sel_method ('user', test_value, 'GB_void', [ ], 1) ;

% GrB_IndexUnaryOp:
fprintf ('\nidxunop    ') ;

% index_unaryop op, A is not iso
zx = 'GB_void z [GB_VLA(zsize)] ; GB_void x [GB_VLA(xsize)] ; ' ;
ca = 'GB_cast_scalar (x, xcode, Ax +(p)*asize, acode, asize) ; ' ;
fn = 'op->idxunop_function (z, x, flipij ? j : i, flipij ? i : j, ythunk) ; ' ;
ke = 'bool keep ; GB_cast_scalar (&keep, GB_BOOL_code, z, zcode, zsize) ;' ;
test_value = [zx ca fn ke] ;
codegen_sel_method ('idxunop', test_value, 'GB_void') ;

% index_unaryop op, A is iso
ca = 'GB_cast_scalar (x, xcode, Ax, acode, asize) ; ' ;
test_value = [zx ca fn ke] ;
codegen_sel_method ('idxunop', test_value, 'GB_void', [ ], 1) ;

% TRIL, TRIU, DIAG, OFFIDIAG, ...
fprintf ('\ntril       ') ;
codegen_sel_method ('tril'      , [ ], 'GB_void' , 'GB_TRIL_SELECTOR'    ) ;
codegen_sel_method ('tril'      , [ ], 'GB_void' , 'GB_TRIL_SELECTOR'    , 1) ;

fprintf ('\ntriu       ') ;
codegen_sel_method ('triu'      , [ ], 'GB_void' , 'GB_TRIU_SELECTOR'    ) ;
codegen_sel_method ('triu'      , [ ], 'GB_void' , 'GB_TRIU_SELECTOR'    , 1) ;

fprintf ('\ndiag       ') ;
codegen_sel_method ('diag'      , [ ], 'GB_void' , 'GB_DIAG_SELECTOR'    ) ;
codegen_sel_method ('diag'      , [ ], 'GB_void' , 'GB_DIAG_SELECTOR'    , 1) ;

fprintf ('\noffdiag    ') ;
codegen_sel_method ('offdiag'   , [ ], 'GB_void' , 'GB_OFFDIAG_SELECTOR' ) ;
codegen_sel_method ('offdiag'   , [ ], 'GB_void' , 'GB_OFFDIAG_SELECTOR' , 1) ;

fprintf ('\nrowindex   ') ;
codegen_sel_method ('rowindex'  , [ ], 'GB_void' , 'GB_ROWINDEX_SELECTOR') ;
codegen_sel_method ('rowindex'  , [ ], 'GB_void' , 'GB_ROWINDEX_SELECTOR', 1) ;

fprintf ('\nrowle      ') ; % was called resize
codegen_sel_method ('rowle'     , [ ], 'GB_void' , 'GB_ROWLE_SELECTOR'   ) ;
codegen_sel_method ('rowle'     , [ ], 'GB_void' , 'GB_ROWLE_SELECTOR'   , 1) ;

fprintf ('\nrowgt      ') ;
codegen_sel_method ('rowgt'     , [ ], 'GB_void' , 'GB_ROWGT_SELECTOR'   ) ;
codegen_sel_method ('rowgt'     , [ ], 'GB_void' , 'GB_ROWGT_SELECTOR'   , 1) ;

fprintf ('\ncolindex   ') ;
codegen_sel_method ('colindex'  , [ ], 'GB_void' , 'GB_COLINDEX_SELECTOR') ;
codegen_sel_method ('colindex'  , [ ], 'GB_void' , 'GB_COLINDEX_SELECTOR', 1) ;

fprintf ('\ncolle      ') ; % was called resize
codegen_sel_method ('colle'     , [ ], 'GB_void' , 'GB_COLLE_SELECTOR'   ) ;
codegen_sel_method ('colle'     , [ ], 'GB_void' , 'GB_COLLE_SELECTOR'   , 1) ;

fprintf ('\ncolgt      ') ;
codegen_sel_method ('colgt'     , [ ], 'GB_void' , 'GB_COLGT_SELECTOR'   ) ;
codegen_sel_method ('colgt'     , [ ], 'GB_void' , 'GB_COLGT_SELECTOR'   , 1) ;

% NONZOMBIE:         name         selector                     type
% phase1: depends on Ai only, so only nonzombie_iso is used
% phase2: use all 15 workers
% TODO NONZOMBIE can use cases 1, 2, 4, 8, 16, other
fprintf ('\nnonzombie  ') ;
test_value = 'bool keep = (Ai [p] >= 0)' ;
codegen_sel_method ('nonzombie', test_value, 'bool'      ) ;
codegen_sel_method ('nonzombie', test_value, 'int8_t'    ) ;
codegen_sel_method ('nonzombie', test_value, 'int16_t'   ) ;
codegen_sel_method ('nonzombie', test_value, 'int32_t'   ) ;
codegen_sel_method ('nonzombie', test_value, 'int64_t'   ) ;
codegen_sel_method ('nonzombie', test_value, 'uint8_t'   ) ;
codegen_sel_method ('nonzombie', test_value, 'uint16_t'  ) ;
codegen_sel_method ('nonzombie', test_value, 'uint32_t'  ) ;
codegen_sel_method ('nonzombie', test_value, 'uint64_t'  ) ;
codegen_sel_method ('nonzombie', test_value, 'float'     ) ;
codegen_sel_method ('nonzombie', test_value, 'double'    ) ;
codegen_sel_method ('nonzombie', test_value, 'GxB_FC32_t') ;
codegen_sel_method ('nonzombie', test_value, 'GxB_FC64_t') ;
codegen_sel_method ('nonzombie', test_value, 'GB_void'   ) ;
codegen_sel_method ('nonzombie', test_value, 'GB_void'   , [ ], 1) ;

%===============================================================================
% select ops with no iso variants
%===============================================================================

% None of these selectops require an iso variant for the selectop worker.  They
% are only used when A is non-iso.  The iso case is handled in GB_selector,
% where either C is all of A, or C is empty.

% TODO NONZERO, EQ_ZERO, NE_THUNK, EQ_THUNK can use cases 1, 2, 4, 8, 16, other

% NONZERO            name         selector       type
fprintf ('\nnonzero    ') ;
codegen_sel_method ('nonzero'  , 'bool keep = (Ax [p])',      'bool'    ) ;
codegen_sel_method ('nonzero'  , 'bool keep = (Ax [p] != 0)', 'int8_t'  ) ;
codegen_sel_method ('nonzero'  , 'bool keep = (Ax [p] != 0)', 'int16_t' ) ;
codegen_sel_method ('nonzero'  , 'bool keep = (Ax [p] != 0)', 'int32_t' ) ;
codegen_sel_method ('nonzero'  , 'bool keep = (Ax [p] != 0)', 'int64_t' ) ;
codegen_sel_method ('nonzero'  , 'bool keep = (Ax [p] != 0)', 'uint8_t' ) ;
codegen_sel_method ('nonzero'  , 'bool keep = (Ax [p] != 0)', 'uint16_t') ;
codegen_sel_method ('nonzero'  , 'bool keep = (Ax [p] != 0)', 'uint32_t') ;
codegen_sel_method ('nonzero'  , 'bool keep = (Ax [p] != 0)', 'uint64_t') ;
codegen_sel_method ('nonzero'  , 'bool keep = (Ax [p] != 0)', 'float'   ) ;
codegen_sel_method ('nonzero'  , 'bool keep = (Ax [p] != 0)', 'double'  ) ;
codegen_sel_method ('nonzero'  , 'bool keep = GB_FC32_ne0 (Ax [p])', 'GxB_FC32_t') ;
codegen_sel_method ('nonzero'  , 'bool keep = GB_FC64_ne0 (Ax [p])', 'GxB_FC64_t') ;
codegen_sel_method ('nonzero'  , 'bool keep = GB_is_nonzero (Ax +((p)*asize), asize)', 'GB_void') ;

% EQ_ZERO            name         selector       type
fprintf ('\neq_zero    ') ;
codegen_sel_method ('eq_zero'  , 'bool keep = (!(Ax [p]))',   'bool'    ) ;
codegen_sel_method ('eq_zero'  , 'bool keep = (Ax [p] == 0)', 'int8_t'  ) ;
codegen_sel_method ('eq_zero'  , 'bool keep = (Ax [p] == 0)', 'int16_t' ) ;
codegen_sel_method ('eq_zero'  , 'bool keep = (Ax [p] == 0)', 'int32_t' ) ;
codegen_sel_method ('eq_zero'  , 'bool keep = (Ax [p] == 0)', 'int64_t' ) ;
codegen_sel_method ('eq_zero'  , 'bool keep = (Ax [p] == 0)', 'uint8_t' ) ;
codegen_sel_method ('eq_zero'  , 'bool keep = (Ax [p] == 0)', 'uint16_t') ;
codegen_sel_method ('eq_zero'  , 'bool keep = (Ax [p] == 0)', 'uint32_t') ;
codegen_sel_method ('eq_zero'  , 'bool keep = (Ax [p] == 0)', 'uint64_t') ;
codegen_sel_method ('eq_zero'  , 'bool keep = (Ax [p] == 0)', 'float'   ) ;
codegen_sel_method ('eq_zero'  , 'bool keep = (Ax [p] == 0)', 'double'  ) ;
codegen_sel_method ('eq_zero'  , 'bool keep = GB_FC32_eq0 (Ax [p])', 'GxB_FC32_t') ;
codegen_sel_method ('eq_zero'  , 'bool keep = GB_FC64_eq0 (Ax [p])', 'GxB_FC64_t') ;
codegen_sel_method ('eq_zero'  , 'bool keep = (!GB_is_nonzero (Ax +((p)*asize), asize))', 'GB_void') ;

% GT_ZERO            name         selector       type
fprintf ('\ngt_zero    ') ;
codegen_sel_method ('gt_zero'  , 'bool keep = (Ax [p] > 0)', 'int8_t'  ) ;
codegen_sel_method ('gt_zero'  , 'bool keep = (Ax [p] > 0)', 'int16_t' ) ;
codegen_sel_method ('gt_zero'  , 'bool keep = (Ax [p] > 0)', 'int32_t' ) ;
codegen_sel_method ('gt_zero'  , 'bool keep = (Ax [p] > 0)', 'int64_t' ) ;
codegen_sel_method ('gt_zero'  , 'bool keep = (Ax [p] > 0)', 'float'   ) ;
codegen_sel_method ('gt_zero'  , 'bool keep = (Ax [p] > 0)', 'double'  ) ;

% GE_ZERO            name         selector       type
fprintf ('\nge_zero    ') ;
codegen_sel_method ('ge_zero'  , 'bool keep = (Ax [p] >= 0)', 'int8_t'  ) ;
codegen_sel_method ('ge_zero'  , 'bool keep = (Ax [p] >= 0)', 'int16_t' ) ;
codegen_sel_method ('ge_zero'  , 'bool keep = (Ax [p] >= 0)', 'int32_t' ) ;
codegen_sel_method ('ge_zero'  , 'bool keep = (Ax [p] >= 0)', 'int64_t' ) ;
codegen_sel_method ('ge_zero'  , 'bool keep = (Ax [p] >= 0)', 'float'   ) ;
codegen_sel_method ('ge_zero'  , 'bool keep = (Ax [p] >= 0)', 'double'  ) ;

% LT_ZERO            name         selector       type
fprintf ('\nlt_zero    ') ;
codegen_sel_method ('lt_zero'  , 'bool keep = (Ax [p] < 0)', 'int8_t'  ) ;
codegen_sel_method ('lt_zero'  , 'bool keep = (Ax [p] < 0)', 'int16_t' ) ;
codegen_sel_method ('lt_zero'  , 'bool keep = (Ax [p] < 0)', 'int32_t' ) ;
codegen_sel_method ('lt_zero'  , 'bool keep = (Ax [p] < 0)', 'int64_t' ) ;
codegen_sel_method ('lt_zero'  , 'bool keep = (Ax [p] < 0)', 'float'   ) ;
codegen_sel_method ('lt_zero'  , 'bool keep = (Ax [p] < 0)', 'double'  ) ;

% LE_ZERO            name         selector       type
fprintf ('\nle_zero    ') ;
codegen_sel_method ('le_zero'  , 'bool keep = (Ax [p] <= 0)', 'int8_t'  ) ;
codegen_sel_method ('le_zero'  , 'bool keep = (Ax [p] <= 0)', 'int16_t' ) ;
codegen_sel_method ('le_zero'  , 'bool keep = (Ax [p] <= 0)', 'int32_t' ) ;
codegen_sel_method ('le_zero'  , 'bool keep = (Ax [p] <= 0)', 'int64_t' ) ;
codegen_sel_method ('le_zero'  , 'bool keep = (Ax [p] <= 0)', 'float'   ) ;
codegen_sel_method ('le_zero'  , 'bool keep = (Ax [p] <= 0)', 'double'  ) ;

% NE_THUNK           name         selector            type
fprintf ('\nne_thunk   ') ;
codegen_sel_method ('ne_thunk'  , 'bool keep = (Ax [p] != thunk)', 'int8_t'  ) ;
codegen_sel_method ('ne_thunk'  , 'bool keep = (Ax [p] != thunk)', 'int16_t' ) ;
codegen_sel_method ('ne_thunk'  , 'bool keep = (Ax [p] != thunk)', 'int32_t' ) ;
codegen_sel_method ('ne_thunk'  , 'bool keep = (Ax [p] != thunk)', 'int64_t' ) ;
codegen_sel_method ('ne_thunk'  , 'bool keep = (Ax [p] != thunk)', 'uint8_t' ) ;
codegen_sel_method ('ne_thunk'  , 'bool keep = (Ax [p] != thunk)', 'uint16_t') ;
codegen_sel_method ('ne_thunk'  , 'bool keep = (Ax [p] != thunk)', 'uint32_t') ;
codegen_sel_method ('ne_thunk'  , 'bool keep = (Ax [p] != thunk)', 'uint64_t') ;
codegen_sel_method ('ne_thunk'  , 'bool keep = (Ax [p] != thunk)', 'float'   ) ;
codegen_sel_method ('ne_thunk'  , 'bool keep = (Ax [p] != thunk)', 'double'  ) ;
codegen_sel_method ('ne_thunk'  , 'bool keep = GB_FC32_ne (Ax [p], thunk)', 'GxB_FC32_t') ;
codegen_sel_method ('ne_thunk'  , 'bool keep = GB_FC64_ne (Ax [p], thunk)', 'GxB_FC64_t') ;
codegen_sel_method ('ne_thunk'  , 'bool keep = (memcmp (Ax +((p)*asize), athunk, asize) != 0)', 'GB_void') ;

% EQ_THUNK           name         selector            type
fprintf ('\neq_thunk   ') ;
codegen_sel_method ('eq_thunk'  , 'bool keep = (Ax [p] == thunk)', 'int8_t'  ) ;
codegen_sel_method ('eq_thunk'  , 'bool keep = (Ax [p] == thunk)', 'int16_t' ) ;
codegen_sel_method ('eq_thunk'  , 'bool keep = (Ax [p] == thunk)', 'int32_t' ) ;
codegen_sel_method ('eq_thunk'  , 'bool keep = (Ax [p] == thunk)', 'int64_t' ) ;
codegen_sel_method ('eq_thunk'  , 'bool keep = (Ax [p] == thunk)', 'uint8_t' ) ;
codegen_sel_method ('eq_thunk'  , 'bool keep = (Ax [p] == thunk)', 'uint16_t') ;
codegen_sel_method ('eq_thunk'  , 'bool keep = (Ax [p] == thunk)', 'uint32_t') ;
codegen_sel_method ('eq_thunk'  , 'bool keep = (Ax [p] == thunk)', 'uint64_t') ;
codegen_sel_method ('eq_thunk'  , 'bool keep = (Ax [p] == thunk)', 'float'   ) ;
codegen_sel_method ('eq_thunk'  , 'bool keep = (Ax [p] == thunk)', 'double'  ) ;
codegen_sel_method ('eq_thunk'  , 'bool keep = GB_FC32_eq (Ax [p], thunk)', 'GxB_FC32_t') ;
codegen_sel_method ('eq_thunk'  , 'bool keep = GB_FC64_eq (Ax [p], thunk)', 'GxB_FC64_t') ;
codegen_sel_method ('eq_thunk'  , 'bool keep = (memcmp (Ax +((p)*asize), athunk, asize) == 0)', 'GB_void') ;

% GT_THUNK           name         selector            type
fprintf ('\ngt_thunk   ') ;
codegen_sel_method ('gt_thunk'  , 'bool keep = (Ax [p] > thunk)', 'int8_t'  ) ;
codegen_sel_method ('gt_thunk'  , 'bool keep = (Ax [p] > thunk)', 'int16_t' ) ;
codegen_sel_method ('gt_thunk'  , 'bool keep = (Ax [p] > thunk)', 'int32_t' ) ;
codegen_sel_method ('gt_thunk'  , 'bool keep = (Ax [p] > thunk)', 'int64_t' ) ;
codegen_sel_method ('gt_thunk'  , 'bool keep = (Ax [p] > thunk)', 'uint8_t' ) ;
codegen_sel_method ('gt_thunk'  , 'bool keep = (Ax [p] > thunk)', 'uint16_t') ;
codegen_sel_method ('gt_thunk'  , 'bool keep = (Ax [p] > thunk)', 'uint32_t') ;
codegen_sel_method ('gt_thunk'  , 'bool keep = (Ax [p] > thunk)', 'uint64_t') ;
codegen_sel_method ('gt_thunk'  , 'bool keep = (Ax [p] > thunk)', 'float'   ) ;
codegen_sel_method ('gt_thunk'  , 'bool keep = (Ax [p] > thunk)', 'double'  ) ;

% GE_THUNK           name         selector            type
fprintf ('\nge_thunk   ') ;
codegen_sel_method ('ge_thunk'  , 'bool keep = (Ax [p] >= thunk)', 'int8_t'  ) ;
codegen_sel_method ('ge_thunk'  , 'bool keep = (Ax [p] >= thunk)', 'int16_t' ) ;
codegen_sel_method ('ge_thunk'  , 'bool keep = (Ax [p] >= thunk)', 'int32_t' ) ;
codegen_sel_method ('ge_thunk'  , 'bool keep = (Ax [p] >= thunk)', 'int64_t' ) ;
codegen_sel_method ('ge_thunk'  , 'bool keep = (Ax [p] >= thunk)', 'uint8_t' ) ;
codegen_sel_method ('ge_thunk'  , 'bool keep = (Ax [p] >= thunk)', 'uint16_t') ;
codegen_sel_method ('ge_thunk'  , 'bool keep = (Ax [p] >= thunk)', 'uint32_t') ;
codegen_sel_method ('ge_thunk'  , 'bool keep = (Ax [p] >= thunk)', 'uint64_t') ;
codegen_sel_method ('ge_thunk'  , 'bool keep = (Ax [p] >= thunk)', 'float'   ) ;
codegen_sel_method ('ge_thunk'  , 'bool keep = (Ax [p] >= thunk)', 'double'  ) ;

% LT_THUNK           name         selector            type
fprintf ('\nlt_thunk   ') ;
codegen_sel_method ('lt_thunk'  , 'bool keep = (Ax [p] < thunk)', 'int8_t'  ) ;
codegen_sel_method ('lt_thunk'  , 'bool keep = (Ax [p] < thunk)', 'int16_t' ) ;
codegen_sel_method ('lt_thunk'  , 'bool keep = (Ax [p] < thunk)', 'int32_t' ) ;
codegen_sel_method ('lt_thunk'  , 'bool keep = (Ax [p] < thunk)', 'int64_t' ) ;
codegen_sel_method ('lt_thunk'  , 'bool keep = (Ax [p] < thunk)', 'uint8_t' ) ;
codegen_sel_method ('lt_thunk'  , 'bool keep = (Ax [p] < thunk)', 'uint16_t') ;
codegen_sel_method ('lt_thunk'  , 'bool keep = (Ax [p] < thunk)', 'uint32_t') ;
codegen_sel_method ('lt_thunk'  , 'bool keep = (Ax [p] < thunk)', 'uint64_t') ;
codegen_sel_method ('lt_thunk'  , 'bool keep = (Ax [p] < thunk)', 'float'   ) ;
codegen_sel_method ('lt_thunk'  , 'bool keep = (Ax [p] < thunk)', 'double'  ) ;

% LE_THUNK           name         selector            type
fprintf ('\nle_thunk   ') ;
codegen_sel_method ('le_thunk'  , 'bool keep = (Ax [p] <= thunk)', 'int8_t'  ) ;
codegen_sel_method ('le_thunk'  , 'bool keep = (Ax [p] <= thunk)', 'int16_t' ) ;
codegen_sel_method ('le_thunk'  , 'bool keep = (Ax [p] <= thunk)', 'int32_t' ) ;
codegen_sel_method ('le_thunk'  , 'bool keep = (Ax [p] <= thunk)', 'int64_t' ) ;
codegen_sel_method ('le_thunk'  , 'bool keep = (Ax [p] <= thunk)', 'uint8_t' ) ;
codegen_sel_method ('le_thunk'  , 'bool keep = (Ax [p] <= thunk)', 'uint16_t') ;
codegen_sel_method ('le_thunk'  , 'bool keep = (Ax [p] <= thunk)', 'uint32_t') ;
codegen_sel_method ('le_thunk'  , 'bool keep = (Ax [p] <= thunk)', 'uint64_t') ;
codegen_sel_method ('le_thunk'  , 'bool keep = (Ax [p] <= thunk)', 'float'   ) ;
codegen_sel_method ('le_thunk'  , 'bool keep = (Ax [p] <= thunk)', 'double'  ) ;

fprintf ('\n') ;

