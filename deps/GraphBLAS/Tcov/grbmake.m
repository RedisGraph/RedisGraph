function grbmake
%GBMAKE compile the GraphBLAS library for statement coverage testing
%
% This function compiles ../Source to create the
% libgraphblas_tcov.so (or *.dylib) library, inserting code code for statement
% coverage testing.  It does not compile the mexFunctions.
%
% See also: grbcover, grbcover_edit

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

if (ispc)
    error ('The tests in Tcov are not ported to Windows') ;
end

% copy the GB_rename.h file
copyfile ('../GraphBLAS/rename/GB_rename.h', 'tmp_include/GB_rename.h') ;

% create the include files and place in tmp_include
hfiles = [ dir('../Include/*') ; ...
           dir('../Source/*.h') ; ...
           dir('../lz4/*.h') ; ...
           dir('../lz4/*.c') ; ...
           dir('../Source/Template') ; ...
           dir('../Source/Generated1/*.h') ; ...
           dir('../Source/Generated2/*.h') ; ] ;
count = grbcover_edit (hfiles, 0, 'tmp_include') ;
fprintf ('hfile count: %d\n', count) ;

% create the C files and place in tmp_source
cfiles = [ dir('../Source/*.c') ; ...
           dir('../Source/Generated1/*.c') ; ...
           dir('../Source/Generated2/*.c') ; ...
           dir('GB_cover_finish.c')
           ] ;
count = grbcover_edit (cfiles, count, 'tmp_source') ;
fprintf ('cfile count: %d\n', count) ;

% create the GB_cover_finish.c file and place in tmp_source
f = fopen ('tmp_source/GB_cover_finish.c', 'w') ;
fprintf (f, '#include "GB.h"\n') ;
fprintf (f, 'int64_t GB_cov [GBCOVER_MAX] ;\n') ;
fprintf (f, 'int GB_cover_max = %d ;\n', count) ;
fclose (f) ;

% compile the libgraphblas_tcov.so library

have_octave = (exist ('OCTAVE_VERSION', 'builtin') == 5) ;
if (have_octave)
    need_rename = false ;
else
    need_rename = ~verLessThan ('matlab', '9.10') ;
end

if (need_rename)
    fprintf ('Rename with -DGBRENAME=1\n') ;
    system (sprintf ('make -j%d RENAME="-DGBRENAME=1"', feature ('numcores'))) ;
else
    system (sprintf ('make -j%d', feature ('numcores'))) ;
end

