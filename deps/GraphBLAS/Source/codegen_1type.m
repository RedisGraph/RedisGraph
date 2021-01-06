function codegen_1type
%CODEGEN_1TYPE create functions for all 13 built-in types
%
% This function creates all files of the form GB_type__*.[ch], including 11
% functions (GB_type__*.c) and one include file, GB_type__include.h.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

fprintf ('\ntypes:\n') ;

f = fopen ('Generated/GB_type__include.h', 'w') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '// GB_type__include.h: definitions for GB_type__*.c\n') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '\n') ;
fprintf (f, '// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.\n') ;
fprintf (f, '// SPDX-License-Identifier: Apache-2.0\n\n') ;
fprintf (f, '// This file has been automatically generated from Generator/GB_type.h') ;
fprintf (f, '\n\n') ;
fclose (f) ;

codegen_1type_template ('bool') ;
codegen_1type_template ('int8_t') ;
codegen_1type_template ('int16_t') ;
codegen_1type_template ('int32_t') ;
codegen_1type_template ('int64_t') ;
codegen_1type_template ('uint8_t') ;
codegen_1type_template ('uint16_t') ;
codegen_1type_template ('uint32_t') ;
codegen_1type_template ('uint64_t') ;
codegen_1type_template ('float') ;
codegen_1type_template ('double') ;
codegen_1type_template ('GxB_FC32_t') ;
codegen_1type_template ('GxB_FC64_t') ;

fprintf ('\n') ;

