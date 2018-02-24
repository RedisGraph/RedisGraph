function gbcover
%GBCOVER compile GraphBLAS for statement coverage testing
%
% This function compiles all of GraphBLAS in ../Source and all GraphBLAS
% mexFunctions in ../Test, and inserts code for statement coverage testing.
%
% See also: gbcover_edit

%  SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
%  http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% create the cover_*template.c files
files = dir ('../Source/Template/*.c') ;
for k = 1:length (files)
    infile  = ['../Source/Template/' files(k).name] ;
    outfile = ['cover_' files(k).name] ;
    gbcover_edit (outfile, infile) ;
end

% create the single cover_gb.c for all other GraphBLAS source files
cfiles = [ dir('gbcover_start.c') ; ...
           dir('../Source/*.c') ; ...
           dir('../Source/Generated/*.c') ; ...
           dir('../Demo/Source/usercomplex.c') ; ...
           dir('../Demo/Source/simple_rand.c') ; ...
           dir('../Demo/Source/random_matrix.c') ; ...
           dir('../Demo/Source/wathen.c') ; ...
           dir('../Demo/Source/mis_check.c') ; ...
           dir('../Demo/Source/mis_score.c') ; ...
           dir('gbcover_finish.c') ];
gbcover_edit ('cover_gb.c', cfiles) ;

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
cfiles = [ dir('cover_gb.c') ; ...
           dir('../Test/GB_mx_*.c') ; ...
           dir('gbcover_util.c') ] ;

% list of *.h and template file dependencies
hfiles = [ dir('../Include/*.h') ; ...
           dir('../Test*.h') ; ...
           dir('../Test/Template/*.c') ; ...
           dir('../Test/Template/*.h') ; ...
           dir('../Source/Template/*.h') ; ...
           dir('../Demo/Include/*.h') ; ...
           dir('*.h') ] ;

% list of include directories
inc = '-I../Include -I../Source -I../Source/Template -I../Source/Generated' ;
inc = [inc ' -I../Test -I../Test/Template -I. -I../Demo/Include'] ;

% gbmake is in ../Test
addpath ../Test
addpath ../Test/spok

% compile GraphBLAS with statement coverage
gbmake ('', '-g -DGBCOVER', mexfunctions, cfiles, hfiles, inc) ;

% This may fail on some systems since it assumes the "cc -E" compiler command
% is available in the system.  That works for Linux and Mac but might not work
% for Windows.  It produces a somewhat readable expanded copy of GraphBLAS with
% statements marked.  See gbcov[k]++ statements; this increments the global
% array GraphBLAS_gbcov in the MATLAB workspace when a test is run.  If
% GraphBLAS_gbcov(k) is zero after the tests finish, then the statement marked
% with gbcov[k-1] has not been tested.  The untested statements are marked
% as "NOT COVERED"
try
    system (sprintf ('cc -E %s cover_gb.c | indent > cover_gb_exp.c', inc)) ;
catch
end

