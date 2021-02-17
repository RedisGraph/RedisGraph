function make (what)
%MAKE compiles the MATLAB interface to GraphBLAS (for testing only)
% and dynamically links it with the libraries in ../build/libgraphblas.
%
% This MATLAB interface to GraphBLAS is meant for testing and development,
% not for general use.
%
% Usage:
%
%   make            % just make what has changed (does not check any changes
%                   % in -lgraphblas, use 'make all' if recompilation is needed
%   make all        % make everything from scratch
%
% GraphBLAS requires an ANSI C11 compliant compiler.  On the Mac, clang 8.0
% suffices.  gcc should be version 4.9.3 or later

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

here = pwd ;
if (ispc)
    here = strrep (here, filesep, '/') ;
end
if (isempty (strfind (here, 'GraphBLAS/Test')))
    % this function should only be done in GraphBLAS/Test
    error ('make should be used in Test directory only') ;
end

fprintf ('\nCompiling GraphBLAS tests:\n') ;

try
    spok (sparse (1)) ;
catch
    here = pwd ;
    cd ('spok') ;
    spok_install ;
    cd (here) ;
end

if (nargin < 1)
    what = '' ;
end

make_all = (isequal (what, 'all')) ;

flags = '-g -R2018a' ;

try
    if (strncmp (computer, 'GLNX', 4))
        % remove -ansi from CFLAGS and replace it with -std=c11
        cc = mex.getCompilerConfigurations ('C', 'Selected') ;
        env = cc.Details.SetEnv ;
        c1 = strfind (env, 'CFLAGS=') ;
        q = strfind (env, '"') ;
        q = q (q > c1) ;
        if (~isempty (c1) && length (q) > 1)
            c2 = q (2) ;
            cflags = env (c1:c2) ;  % the CFLAGS="..." string
            ansi = strfind (cflags, '-ansi') ;
            if (~isempty (ansi))
                cflags = [cflags(1:ansi-1) '-std=c11' cflags(ansi+5:end)] ;
                flags = [flags ' ' cflags] ;
                fprintf ('compiling with -std=c11 instead of default -ansi\n') ;
            end
        end
    end
catch
end

mexfunctions = dir ('GB_mex_*.c') ;
cfiles = [ dir('GB_mx_*.c') ; ...
           dir('../Demo/Source/bfs5m.c') ; ...
           dir('../Demo/Source/drowscale.c') ; ...
           dir('../Demo/Source/dpagerank.c') ; ...
           dir('../Demo/Source/dpagerank2.c') ; ...
           dir('../Demo/Source/ipagerank.c') ; ...
           dir('../Demo/Source/irowscale.c') ; ...
           dir('../Demo/Source/isequal.c') ; ...
           dir('../Demo/Source/mis_check.c') ; ...
           dir('../Demo/Source/mis_score.c') ; ...
           dir('../Demo/Source/wathen.c') ; ...
           dir('../Demo/Source/random_matrix.c') ; ...
           dir('../Demo/Source/simple_rand.c') ; ...
           dir('../Demo/Source/prand.c') ; ...
           dir('../Demo/Source/simple_timer.c') ; ...
           dir('../Demo/Source/tricount.c') ; ...
           dir('../Demo/Source/usercomplex.c') ] ;

hfiles = [ dir('*.h') ; dir('Template/*.c') ; dir('../Demo/Include/*.h') ] ;
inc = '-ITemplate -I../Include -I../Source -I../Source/Template -I../Demo/Include' ;

if (ismac)
    % Mac (do 'make install' for GraphBLAS first)
    libraries = '-L/usr/local/lib -lgraphblas' ; % -lomp' ;
%   flags = [ flags   ' CFLAGS="$CXXFLAGS -Xpreprocessor -fopenmp" ' ] ;
%   flags = [ flags ' CXXFLAGS="$CXXFLAGS -Xpreprocessor -fopenmp" ' ] ;
%   flags = [ flags  ' LDFLAGS="$LDFLAGS  -fopenmp"' ] ;
elseif (ispc)
    % Windows
    libraries = '-L../build/Release -L. -lgraphblas' ;
    flags = [ flags ' CFLAGS="$CXXFLAGS -wd\"4244\" -wd\"4146\" -wd\"4217\" -wd\"4286\" -wd\"4018\" -wd\"4996\" -wd\"4047\" -wd\"4554\"" '] ;
else
    % Linux
    libraries = '-L../build -L. -lgraphblas' ;
    flags = [ flags   ' CFLAGS="$CXXFLAGS -fopenmp -fPIC -Wno-pragmas" '] ;
    flags = [ flags ' CXXFLAGS="$CXXFLAGS -fopenmp -fPIC -Wno-pragmas" '] ;
    flags = [ flags  ' LDFLAGS="$LDFLAGS  -fopenmp -fPIC" '] ;
end

%-------------------------------------------------------------------------------

dryrun = false ;

% Find the last modification time of any hfile.
% These are #include'd into source files.
htime = 0 ;
for k = 1:length (hfiles)
    t = datenum (hfiles (k).date) ;
    htime = max (htime, t) ;
end

if (ispc)
    obj_extension = '.obj' ;
else
    obj_extension = '.o' ;
end

% compile any source files that need compiling
any_c_compiled = false ;
objlist = '' ;
for k = 1:length (cfiles)

    % get the full cfile filename and  modification time
    cfile = [(cfiles (k).folder) filesep (cfiles (k).name)] ;
    tc = datenum (cfiles(k).date) ;

    % get the object file name
    ofile = cfiles(k).name ;
    objfile = [ ofile(1:end-2) obj_extension ] ;

    % get the object file modification time
    ofiles {k} = objfile ;
    objlist = [ objlist ' ' objfile ] ;
    dobj = dir (objfile) ;
    if (isempty (dobj))
        % there is no object file; the cfile must be compiled
        tobj = 0 ;
    else
        tobj = datenum (dobj.date) ;
    end

    % compile the cfile if it is newer than its object file, or any hfile
    if (make_all || tc > tobj || htime > tobj)
        % compile the cfile
        fprintf ('.', cfile) ;
        % fprintf ('%s\n', cfile) ;
        mexcmd = sprintf ('mex -c %s -silent %s %s', flags, inc, cfile) ;
        if (dryrun)
            fprintf ('%s\n', mexcmd) ;
        else
            % fprintf ('%s\n', mexcmd) ;
            eval (mexcmd) ;
        end
        any_c_compiled = true ;
    end
end

% compile the mexFunctions
for k = 1:length (mexfunctions)

    % get the mexFunction filename and modification time
    mexfunc = mexfunctions (k).name ;
    mexfunction = [(mexfunctions (k).folder) filesep mexfunc] ;
    tc = datenum (mexfunctions(k).date) ;

    % get the compiled mexFunction modification time
    mexfunction_compiled = [ mexfunc(1:end-2) '.' mexext ] ;
    dobj = dir (mexfunction_compiled) ;
    if (isempty (dobj))
        % there is no compiled mexFunction; it must be compiled
        tobj = 0 ;
    else
        tobj = datenum (dobj.date) ;
    end

    % compile if it is newer than its object file, or if any cfile was compiled
    if (make_all || tc > tobj || any_c_compiled)
        % compile the mexFunction
        mexcmd = sprintf ('mex -silent %s %s %s %s %s', ...
            flags, inc, mexfunction, objlist, libraries) ;
        fprintf (':') ;
        % fprintf ('%s\n', mexfunction) ;
        if (dryrun)
            fprintf ('%s\n', mexcmd) ;
        else
            % fprintf ('%s\n', mexcmd) ;
            eval (mexcmd) ;
        end
    end
end

% compile GB_spones_mex
mex -O -R2018a GB_spones_mex.c

% load the library
if (ispc)
    cd ../build/Release
    GrB (1)
    cd ../../Test
    pwd
end

