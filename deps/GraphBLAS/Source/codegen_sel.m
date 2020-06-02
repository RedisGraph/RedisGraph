function codegen_sel
%CODEGEN_SEL create functions for all selection operators
%
% This function creates all files of the form GB_sel__*.c,
% and the include file GB_sel__include.h.

fprintf ('\nselection operators:\n') ;

f = fopen ('Generated/GB_sel__include.h', 'w') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '// GB_sel__include.h: definitions for GB_sel__*.c\n') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '\n') ;
fprintf (f, '// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.\n') ;
fprintf (f, '// http://suitesparse.com   See GraphBLAS/Doc/License.txargt for license.\n') ;
fprintf (f, '\n') ;
fprintf (f, '// This file has been automatically generated from Generator/GB_sel.h') ;
fprintf (f, '\n\n') ;
fclose (f) ;

%-------------------------------------------------------------------------------

% USER:
fprintf ('\nuser       ') ;
codegen_sel_method ('user',  ...
    [ 'user_select (' ...
      ' flipij ? j : Ai[p], ' ...
      ' flipij ? Ai[p] : j, ' ...
      ' flipij ? avdim : avlen, ' ...
      ' flipij ? avlen : avdim, Ax +((p)*asize), xthunk)' ] , 'GB_void') ;

% TRIL, TRIU, DIAG, OFFIDIAG, RESIZE:
fprintf ('\ntril       ') ;
codegen_sel_method ('tril'      , [ ], 'GB_void' , 'GB_TRIL_SELECTOR'    ) ;
fprintf ('\ntriu       ') ;
codegen_sel_method ('triu'      , [ ], 'GB_void' , 'GB_TRIU_SELECTOR'    ) ;
fprintf ('\ndiag       ') ;
codegen_sel_method ('diag'      , [ ], 'GB_void' , 'GB_DIAG_SELECTOR'    ) ;
fprintf ('\noffdiag    ') ;
codegen_sel_method ('offdiag'   , [ ], 'GB_void' , 'GB_OFFDIAG_SELECTOR' ) ;
fprintf ('\nresize     ') ;
codegen_sel_method ('resize'    , [ ], 'GB_void' , 'GB_RESIZE_SELECTOR'  ) ;

% NONZOMBIE:         name         selector                     type
% phase1: depends on Ai only, so only nonzombie_any is used
% phase2: use all 12 workers
fprintf ('\nnonzombie  ') ;
codegen_sel_method ('nonzombie', 'GB_IS_NOT_ZOMBIE (Ai [p])', 'bool'    ) ;
codegen_sel_method ('nonzombie', 'GB_IS_NOT_ZOMBIE (Ai [p])', 'int8_t'  ) ;
codegen_sel_method ('nonzombie', 'GB_IS_NOT_ZOMBIE (Ai [p])', 'int16_t' ) ;
codegen_sel_method ('nonzombie', 'GB_IS_NOT_ZOMBIE (Ai [p])', 'int32_t' ) ;
codegen_sel_method ('nonzombie', 'GB_IS_NOT_ZOMBIE (Ai [p])', 'int64_t' ) ;
codegen_sel_method ('nonzombie', 'GB_IS_NOT_ZOMBIE (Ai [p])', 'uint8_t' ) ;
codegen_sel_method ('nonzombie', 'GB_IS_NOT_ZOMBIE (Ai [p])', 'uint16_t') ;
codegen_sel_method ('nonzombie', 'GB_IS_NOT_ZOMBIE (Ai [p])', 'uint32_t') ;
codegen_sel_method ('nonzombie', 'GB_IS_NOT_ZOMBIE (Ai [p])', 'uint64_t') ;
codegen_sel_method ('nonzombie', 'GB_IS_NOT_ZOMBIE (Ai [p])', 'float'   ) ;
codegen_sel_method ('nonzombie', 'GB_IS_NOT_ZOMBIE (Ai [p])', 'double'  ) ;
codegen_sel_method ('nonzombie', 'GB_IS_NOT_ZOMBIE (Ai [p])', 'GB_void' ) ;

% NONZERO            name         selector       type
fprintf ('\nnonzero    ') ;
codegen_sel_method ('nonzero'  , 'Ax [p]',      'bool'    ) ;
codegen_sel_method ('nonzero'  , 'Ax [p] != 0', 'int8_t'  ) ;
codegen_sel_method ('nonzero'  , 'Ax [p] != 0', 'int16_t' ) ;
codegen_sel_method ('nonzero'  , 'Ax [p] != 0', 'int32_t' ) ;
codegen_sel_method ('nonzero'  , 'Ax [p] != 0', 'int64_t' ) ;
codegen_sel_method ('nonzero'  , 'Ax [p] != 0', 'uint8_t' ) ;
codegen_sel_method ('nonzero'  , 'Ax [p] != 0', 'uint16_t') ;
codegen_sel_method ('nonzero'  , 'Ax [p] != 0', 'uint32_t') ;
codegen_sel_method ('nonzero'  , 'Ax [p] != 0', 'uint64_t') ;
codegen_sel_method ('nonzero'  , 'Ax [p] != 0', 'float'   ) ;
codegen_sel_method ('nonzero'  , 'Ax [p] != 0', 'double'  ) ;
codegen_sel_method ('nonzero'  , ...
                    'GB_is_nonzero (Ax +((p)*asize), asize)', 'GB_void') ;

% EQ_ZERO            name         selector       type
fprintf ('\neq_zero    ') ;
codegen_sel_method ('eq_zero'  , '!(Ax [p])',   'bool'    ) ;
codegen_sel_method ('eq_zero'  , 'Ax [p] == 0', 'int8_t'  ) ;
codegen_sel_method ('eq_zero'  , 'Ax [p] == 0', 'int16_t' ) ;
codegen_sel_method ('eq_zero'  , 'Ax [p] == 0', 'int32_t' ) ;
codegen_sel_method ('eq_zero'  , 'Ax [p] == 0', 'int64_t' ) ;
codegen_sel_method ('eq_zero'  , 'Ax [p] == 0', 'uint8_t' ) ;
codegen_sel_method ('eq_zero'  , 'Ax [p] == 0', 'uint16_t') ;
codegen_sel_method ('eq_zero'  , 'Ax [p] == 0', 'uint32_t') ;
codegen_sel_method ('eq_zero'  , 'Ax [p] == 0', 'uint64_t') ;
codegen_sel_method ('eq_zero'  , 'Ax [p] == 0', 'float'   ) ;
codegen_sel_method ('eq_zero'  , 'Ax [p] == 0', 'double'  ) ;
codegen_sel_method ('eq_zero'  , ...
                    '!GB_is_nonzero (Ax +((p)*asize), asize)', 'GB_void') ;

% GT_ZERO            name         selector       type
fprintf ('\ngt_zero    ') ;
codegen_sel_method ('gt_zero'  , 'Ax [p] > 0', 'int8_t'  ) ;
codegen_sel_method ('gt_zero'  , 'Ax [p] > 0', 'int16_t' ) ;
codegen_sel_method ('gt_zero'  , 'Ax [p] > 0', 'int32_t' ) ;
codegen_sel_method ('gt_zero'  , 'Ax [p] > 0', 'int64_t' ) ;
codegen_sel_method ('gt_zero'  , 'Ax [p] > 0', 'float'   ) ;
codegen_sel_method ('gt_zero'  , 'Ax [p] > 0', 'double'  ) ;

% GE_ZERO            name         selector       type
fprintf ('\nge_zero    ') ;
codegen_sel_method ('ge_zero'  , 'Ax [p] >= 0', 'int8_t'  ) ;
codegen_sel_method ('ge_zero'  , 'Ax [p] >= 0', 'int16_t' ) ;
codegen_sel_method ('ge_zero'  , 'Ax [p] >= 0', 'int32_t' ) ;
codegen_sel_method ('ge_zero'  , 'Ax [p] >= 0', 'int64_t' ) ;
codegen_sel_method ('ge_zero'  , 'Ax [p] >= 0', 'float'   ) ;
codegen_sel_method ('ge_zero'  , 'Ax [p] >= 0', 'double'  ) ;

% LT_ZERO            name         selector       type
fprintf ('\nlt_zero    ') ;
codegen_sel_method ('lt_zero'  , 'Ax [p] < 0', 'int8_t'  ) ;
codegen_sel_method ('lt_zero'  , 'Ax [p] < 0', 'int16_t' ) ;
codegen_sel_method ('lt_zero'  , 'Ax [p] < 0', 'int32_t' ) ;
codegen_sel_method ('lt_zero'  , 'Ax [p] < 0', 'int64_t' ) ;
codegen_sel_method ('lt_zero'  , 'Ax [p] < 0', 'float'   ) ;
codegen_sel_method ('lt_zero'  , 'Ax [p] < 0', 'double'  ) ;

% LE_ZERO            name         selector       type
fprintf ('\nle_zero    ') ;
codegen_sel_method ('le_zero'  , 'Ax [p] <= 0', 'int8_t'  ) ;
codegen_sel_method ('le_zero'  , 'Ax [p] <= 0', 'int16_t' ) ;
codegen_sel_method ('le_zero'  , 'Ax [p] <= 0', 'int32_t' ) ;
codegen_sel_method ('le_zero'  , 'Ax [p] <= 0', 'int64_t' ) ;
codegen_sel_method ('le_zero'  , 'Ax [p] <= 0', 'float'   ) ;
codegen_sel_method ('le_zero'  , 'Ax [p] <= 0', 'double'  ) ;

% NE_THUNK           name         selector            type
fprintf ('\nne_thunk   ') ;
codegen_sel_method ('ne_thunk'  , 'Ax [p] != thunk', 'int8_t'  ) ;
codegen_sel_method ('ne_thunk'  , 'Ax [p] != thunk', 'int16_t' ) ;
codegen_sel_method ('ne_thunk'  , 'Ax [p] != thunk', 'int32_t' ) ;
codegen_sel_method ('ne_thunk'  , 'Ax [p] != thunk', 'int64_t' ) ;
codegen_sel_method ('ne_thunk'  , 'Ax [p] != thunk', 'uint8_t' ) ;
codegen_sel_method ('ne_thunk'  , 'Ax [p] != thunk', 'uint16_t') ;
codegen_sel_method ('ne_thunk'  , 'Ax [p] != thunk', 'uint32_t') ;
codegen_sel_method ('ne_thunk'  , 'Ax [p] != thunk', 'uint64_t') ;
codegen_sel_method ('ne_thunk'  , 'Ax [p] != thunk', 'float'   ) ;
codegen_sel_method ('ne_thunk'  , 'Ax [p] != thunk', 'double'  ) ;
codegen_sel_method ('ne_thunk'  , ...
                    'memcmp (Ax +((p)*asize), xthunk, asize) != 0', 'GB_void') ;

% EQ_THUNK           name         selector            type
fprintf ('\neq_thunk   ') ;
codegen_sel_method ('eq_thunk'  , 'Ax [p] == thunk', 'int8_t'  ) ;
codegen_sel_method ('eq_thunk'  , 'Ax [p] == thunk', 'int16_t' ) ;
codegen_sel_method ('eq_thunk'  , 'Ax [p] == thunk', 'int32_t' ) ;
codegen_sel_method ('eq_thunk'  , 'Ax [p] == thunk', 'int64_t' ) ;
codegen_sel_method ('eq_thunk'  , 'Ax [p] == thunk', 'uint8_t' ) ;
codegen_sel_method ('eq_thunk'  , 'Ax [p] == thunk', 'uint16_t') ;
codegen_sel_method ('eq_thunk'  , 'Ax [p] == thunk', 'uint32_t') ;
codegen_sel_method ('eq_thunk'  , 'Ax [p] == thunk', 'uint64_t') ;
codegen_sel_method ('eq_thunk'  , 'Ax [p] == thunk', 'float'   ) ;
codegen_sel_method ('eq_thunk'  , 'Ax [p] == thunk', 'double'  ) ;
codegen_sel_method ('eq_thunk'  , ...
                    'memcmp (Ax +((p)*asize), xthunk, asize) == 0', 'GB_void') ;

% GT_THUNK           name         selector            type
fprintf ('\ngt_thunk   ') ;
codegen_sel_method ('gt_thunk'  , 'Ax [p] > thunk', 'int8_t'  ) ;
codegen_sel_method ('gt_thunk'  , 'Ax [p] > thunk', 'int16_t' ) ;
codegen_sel_method ('gt_thunk'  , 'Ax [p] > thunk', 'int32_t' ) ;
codegen_sel_method ('gt_thunk'  , 'Ax [p] > thunk', 'int64_t' ) ;
codegen_sel_method ('gt_thunk'  , 'Ax [p] > thunk', 'uint8_t' ) ;
codegen_sel_method ('gt_thunk'  , 'Ax [p] > thunk', 'uint16_t') ;
codegen_sel_method ('gt_thunk'  , 'Ax [p] > thunk', 'uint32_t') ;
codegen_sel_method ('gt_thunk'  , 'Ax [p] > thunk', 'uint64_t') ;
codegen_sel_method ('gt_thunk'  , 'Ax [p] > thunk', 'float'   ) ;
codegen_sel_method ('gt_thunk'  , 'Ax [p] > thunk', 'double'  ) ;

% GE_THUNK           name         selector            type
fprintf ('\nge_thunk   ') ;
codegen_sel_method ('ge_thunk'  , 'Ax [p] >= thunk', 'int8_t'  ) ;
codegen_sel_method ('ge_thunk'  , 'Ax [p] >= thunk', 'int16_t' ) ;
codegen_sel_method ('ge_thunk'  , 'Ax [p] >= thunk', 'int32_t' ) ;
codegen_sel_method ('ge_thunk'  , 'Ax [p] >= thunk', 'int64_t' ) ;
codegen_sel_method ('ge_thunk'  , 'Ax [p] >= thunk', 'uint8_t' ) ;
codegen_sel_method ('ge_thunk'  , 'Ax [p] >= thunk', 'uint16_t') ;
codegen_sel_method ('ge_thunk'  , 'Ax [p] >= thunk', 'uint32_t') ;
codegen_sel_method ('ge_thunk'  , 'Ax [p] >= thunk', 'uint64_t') ;
codegen_sel_method ('ge_thunk'  , 'Ax [p] >= thunk', 'float'   ) ;
codegen_sel_method ('ge_thunk'  , 'Ax [p] >= thunk', 'double'  ) ;

% LT_THUNK           name         selector            type
fprintf ('\nlt_thunk   ') ;
codegen_sel_method ('lt_thunk'  , 'Ax [p] < thunk', 'int8_t'  ) ;
codegen_sel_method ('lt_thunk'  , 'Ax [p] < thunk', 'int16_t' ) ;
codegen_sel_method ('lt_thunk'  , 'Ax [p] < thunk', 'int32_t' ) ;
codegen_sel_method ('lt_thunk'  , 'Ax [p] < thunk', 'int64_t' ) ;
codegen_sel_method ('lt_thunk'  , 'Ax [p] < thunk', 'uint8_t' ) ;
codegen_sel_method ('lt_thunk'  , 'Ax [p] < thunk', 'uint16_t') ;
codegen_sel_method ('lt_thunk'  , 'Ax [p] < thunk', 'uint32_t') ;
codegen_sel_method ('lt_thunk'  , 'Ax [p] < thunk', 'uint64_t') ;
codegen_sel_method ('lt_thunk'  , 'Ax [p] < thunk', 'float'   ) ;
codegen_sel_method ('lt_thunk'  , 'Ax [p] < thunk', 'double'  ) ;

% LE_THUNK           name         selector            type
fprintf ('\nle_thunk   ') ;
codegen_sel_method ('le_thunk'  , 'Ax [p] <= thunk', 'int8_t'  ) ;
codegen_sel_method ('le_thunk'  , 'Ax [p] <= thunk', 'int16_t' ) ;
codegen_sel_method ('le_thunk'  , 'Ax [p] <= thunk', 'int32_t' ) ;
codegen_sel_method ('le_thunk'  , 'Ax [p] <= thunk', 'int64_t' ) ;
codegen_sel_method ('le_thunk'  , 'Ax [p] <= thunk', 'uint8_t' ) ;
codegen_sel_method ('le_thunk'  , 'Ax [p] <= thunk', 'uint16_t') ;
codegen_sel_method ('le_thunk'  , 'Ax [p] <= thunk', 'uint32_t') ;
codegen_sel_method ('le_thunk'  , 'Ax [p] <= thunk', 'uint64_t') ;
codegen_sel_method ('le_thunk'  , 'Ax [p] <= thunk', 'float'   ) ;
codegen_sel_method ('le_thunk'  , 'Ax [p] <= thunk', 'double'  ) ;

fprintf ('\n') ;

