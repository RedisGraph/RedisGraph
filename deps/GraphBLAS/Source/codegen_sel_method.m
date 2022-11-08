function codegen_sel_method (opname, func, atype, kind, iso)
%CODEGEN_SEL_METHOD create a selection function, C = select (A,thunk)
%
% codegen_sel_method (opname, func, atype, kind)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

if (nargin < 4)
    kind = [ ] ;
end
if (nargin < 5)
    iso = 0 ;
end

is_entry_selector = isempty (kind) ;
is_nonzombie_selector = isequal (opname, 'nonzombie') ;
is_idxunop_selector = codegen_contains (opname, 'idxunop') ;

f = fopen ('control.m4', 'w') ;

[aname, ~, ~] = codegen_type (atype) ;
if (iso)
    aname = 'iso' ;
end

name = sprintf ('%s_%s', opname, aname) ;

if (test_contains (opname, 'col'))
    % only bitmap selector is used
    enable_phase1 = false ;
    enable_phase2 = false ;
else
    enable_phase1 = iso || (is_entry_selector && ~is_nonzombie_selector) ;
    enable_phase2 = true ;
end

fprintf (f, 'define(`GB_iso_select'', `%d'')\n', iso) ;

% function names
if (enable_phase1)
    fprintf (f, 'define(`_sel_phase1'', `_sel_phase1__%s'')\n', name) ;
else
    fprintf (f, 'define(`_sel_phase1'', `_sel_phase1__(none)'')\n') ;
end
if (enable_phase2)
    fprintf (f, 'define(`_sel_phase2'', `_sel_phase2__%s'')\n', name) ;
else
    fprintf (f, 'define(`_sel_phase2'', `_sel_phase2__(none)'')\n', name) ;
end

% zcode, zsize, xcode, xsize
zxtype = '' ;
if (is_idxunop_selector)
    zcode = 'GB_Type_code zcode = op->ztype->code, ' ;
    xcode = 'xcode = op->xtype->code, ' ;
    acode = 'acode = A->type->code ; ' ;
    sizes = 'size_t zsize = op->ztype->size, xsize = op->xtype->size ;' ;
    zxtype = [zcode xcode acode sizes] ;
end
fprintf (f, 'define(`GB_get_zxtypes'', `%s'')\n', zxtype) ;

if isequal (opname, 'nonzombie')
    % no bitmap selectors for nonzombie selectors
    fprintf (f, 'define(`_sel_bitmap'', `_sel_bitmap__(none)'')\n') ;
    fprintf (f, 'define(`if_bitmap'', `#if 0'')\n') ;
    fprintf (f, 'define(`endif_bitmap'', `#endif'')\n') ;
else
    fprintf (f, 'define(`_sel_bitmap'', `_sel_bitmap__%s'')\n', name) ;
    fprintf (f, 'define(`if_bitmap'', `'')\n') ;
    fprintf (f, 'define(`endif_bitmap'', `'')\n') ;
end

% the type of A (no typecasting)
fprintf (f, 'define(`GB_atype'', `%s'')\n', atype) ;

% create the operator to test the numerical values of the entries
if (isempty (func))
    fprintf (f, 'define(`GB_setup'', `'')\n') ;
    fprintf (f, 'define(`GB_test_value_of_entry'', `(no test; %s ignores values)'')\n', opname) ;
elseif (iscell (func))
    fprintf (f, 'define(`GB_setup'', `%s'')\n', func {1}) ;
    fprintf (f, 'define(`GB_test_value_of_entry'', `%s'')\n', func {2}) ;
else
    fprintf (f, 'define(`GB_setup'', `'')\n') ;
    fprintf (f, 'define(`GB_test_value_of_entry'', `%s'')\n', func) ;
end

if (is_entry_selector || is_nonzombie_selector)
    fprintf (f, 'define(`GB_kind'', `#define GB_ENTRY_SELECTOR'')\n') ;
else
    fprintf (f, 'define(`GB_kind'', `#define %s'')\n', kind) ;
end

% get scalar thunk
if (~isequal (atype, 'GB_void') && ~isempty (strfind (opname, 'thunk')))
    fprintf (f, 'define(`GB_get_thunk'', `%s thunk = (*athunk) ;'')\n', atype) ;
else
    fprintf (f, 'define(`GB_get_thunk'', `'')\n') ;
end

% enable phase1
if (enable_phase1)
    % nonzombie: phase1 uses a single worker: GB_sel_phase1__nonzombie_iso
    fprintf (f, 'define(`if_phase1'', `'')\n') ;
    fprintf (f, 'define(`endif_phase1'', `'')\n') ;
else
    fprintf (f, 'define(`if_phase1'', `#if 0'')\n') ;
    fprintf (f, 'define(`endif_phase1'', `#endif'')\n') ;
end

% enable phase2
if (enable_phase2)
    fprintf (f, 'define(`if_phase2'', `'')\n') ;
    fprintf (f, 'define(`endif_phase2'', `'')\n') ;
else
    fprintf (f, 'define(`if_phase2'', `#if 0'')\n') ;
    fprintf (f, 'define(`endif_phase2'', `#endif'')\n') ;
end

% for phase2: copy the numerical value of the entry
if (iso)
    % A is iso
    fprintf (f, 'define(`GB_select_entry'', `/* assignment skipped, C and A are iso */'')\n') ;
elseif (isequal (opname, 'eq_zero'))
    % create C as iso for all EQ_ZERO ops even when A is not iso, with iso value zero
    fprintf (f, 'define(`GB_select_entry'', `/* assignment skipped, C is iso with all entries zero */'')\n') ;
elseif (isequal (opname, 'eq_thunk'))
    % create C as iso for all EQ_THUNK ops even when A is not iso, with iso value athunk
    fprintf (f, 'define(`GB_select_entry'', `/* assignment skipped, C is iso with all entries equal to thunk */'')\n') ;
elseif (isequal (opname, 'nonzero') && isequal (atype, 'bool'))
    % create C as iso for NONZERO_BOOL even when A is not iso, with iso value true
    fprintf (f, 'define(`GB_select_entry'', `/* assignment skipped, C is iso with all entries true */'')\n') ;
elseif (isequal (atype, 'GB_void'))
    fprintf (f, 'define(`GB_select_entry'', `memcpy (Cx +((pC)*asize), Ax +((pA)*asize), asize)'')\n') ;
else
    fprintf (f, 'define(`GB_select_entry'', `Cx [pC] = Ax [pA]'')\n') ;
end

fclose (f) ;

% construct the *.c file
cmd = sprintf (...
'cat control.m4 Generator/GB_sel.c | m4 | tail -n +18 > Generated1/GB_sel__%s.c', ...
name) ;
fprintf ('.') ;
system (cmd) ;

% append to the *.h file
cmd = sprintf (...
'cat control.m4 Generator/GB_sel.h | m4 | tail -n +18 >> Generated1/GB_sel__include.h') ;
system (cmd) ;

delete ('control.m4') ;


