function gbcmake
%GBCMAKE compile GraphBLAS mexFunctions with coverage tests

% get a list of the GraphBLAS mexFunctions
mexfunctions = dir ('../Test/GB_mex_*.c') ;

% remove GB_mex_tricount from the list of mexFunctions
nmex = length (mexfunctions) ;
for k = nmex:-1:1
    if (isequal (mexfunctions (k).name, 'GB_mex_tricount.c'))
        mexfunctions (k) = [ ] ;
    end
end

% list of C files to compile
cfiles = [ dir('tmp_source/*.c') ; ...
           dir('../Test/GB_mx_*.c') ; ...
           dir('GB_cover_util.c') ] ;

% list of *.h and template file dependencies
hfiles = [ dir('tmp_include/*.c') ; ...
           dir('tmp_include/*.h') ; ...
           dir('../Include/*.h') ; ...
           dir('../Test/*.h') ; ...
           dir('../Test/Template/*.c') ; ...
           dir('../Demo/Include/*.h') ] ;

% list of include directories
inc = ...
'-Itmp_include -I../Include -I../Test -I../Test/Template -I../Demo/Include' ;

% gbmake is in ../Test
addpath ../Test
addpath ../Test/spok

% compile GraphBLAS with statement coverage
gbmake ('', '-g -DGBCOVER', mexfunctions, cfiles, hfiles, inc) ;


