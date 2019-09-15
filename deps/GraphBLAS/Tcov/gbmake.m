function gbmake
%GBMAKE compile the GraphBLAS library for statement coverage testing
%
% This function compiles ../Source and ../Demo to create the
% libgraphblas_tcov.so (or *.dylib) library, inserting code code for statement
% coverage testing.  It does not compile the mexFunctions.
%
% See also: gbcover, gbcover_edit

%  SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
%  http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% copy the GraphBLAS.h file
copyfile ('../Include/GraphBLAS.h', 'tmp_include/GraphBLAS.h') ;

% create the include files and place in tmp_include
hfiles = [ dir('../Demo/Include') ; ...
           dir('../Source/*.h') ; ...
           dir('../Source/Template') ; ...
           dir('../Source/Generated/*.h') ; ...
           dir('../Source/Generator/GB_AxB.*') ; ] ;
count = gbcover_edit (hfiles, 0, 'tmp_include') ;
fprintf ('hfile count: %d\n', count) ;

% create the C files and place in tmp_source
cfiles = [ dir('../Source/*.c') ; ...
           dir('../Source/Generated/*.c') ; ...

           dir('GB_cover_finish.c')
           ] ;
count = gbcover_edit (cfiles, count, 'tmp_source') ;
fprintf ('cfile count: %d\n', count) ;

% create the GB_cover_finish.c file and place in tmp_source
f = fopen ('tmp_source/GB_cover_finish.c', 'w') ;
fprintf (f, '#include "GB.h"\n') ;
fprintf (f, 'int64_t GB_cov [GBCOVER_MAX] ;\n') ;
fprintf (f, 'int GB_cover_max = %d ;\n', count) ;
fclose (f) ;

% compile the libgraphblas_tcov.so library

system (sprintf ('make -j%d', feature ('numcores'))) ;

