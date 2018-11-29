function gbmake (what, flags, mexfunctions, cfiles, hfiles, inc)
%GBMAKE compiles the MATLAB interface to GraphBLAS (for testing only).
% Also compiles the functions in the Demo folder for use in testing.
%
% This MATLAB interface to GraphBLAS is meant for testing and development,
% not for general use.
%
% Usage:
%
%   gbmake (what, flags, mexfunctions, cfiles, hfiles, inc)
% 
% All arguments are optional, so that gbmake with no arguments compiles
% the GraphBLAS MATLAB interface with defaults.
%
% what: if empty, compile the code; 'clean' removes object files, 'distclean'
%       removes object files and compiled mexFunctions
%
% flags: defaults to '-O'
% mexfunctions: list of source files of mexFunctions
% cfiles: list of C source files
% hfiles: list of include files
% inc: -I arguments
%
% GraphBLAS requires an ANSI C11 compliant compiler.  On the Mac, clang 8.0
% suffices.  gcc should be version 4.9.3 or later

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (~isempty (strfind (pwd, 'Tcov')) && nargin ~= 6)
    % if the directory is Tcov, and if so assert nargin == 6
    error ('gbmake should not be used in Tcov directory; use testcov instead') ;
end

if (isempty (strfind (pwd, 'Test')) && nargin == 0)
    % gbmake with no arguments should only be done in GraphBLAS/Test
    error ('gbmake (with no arguments) should be used in Test directory only') ;
end

fprintf ('\nCompiling GraphBLAS tests\nplease wait [') ;

if (nargin < 2)
    flags = '-O' ;
end

flags = [flags ' -largeArrayDims'] ;

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
end

if (nargin < 3)
    mexfunctions = dir ('GB_mex_*.c') ;
end

if (nargin < 4)
    cfiles = [ dir('../Source/*.c') ; dir('../Source/Generated/*.c') ; ...
               dir('GB_mx_*.c') ; dir('../Demo/Source/*.c') ] ;
end

if (nargin < 5)
    hfiles = [ dir('../Include/*.h') ; ...
               dir('*.h') ; ...
               dir('Template/*.c') ; ...
               dir('Template/*.h') ; ...
               dir('../Source/*.h') ; ...
               dir('../Source/Generated/*.h') ; ...
               dir('../Source/Generator/*.c') ; ...
               dir('../Demo/Include/*.h') ; ...
               dir('../Source/Template/*.h') ; ...
               dir('../Source/Template/*.c') ] ;
end

if (nargin < 6)
    inc = '-ITemplate -I../Include -I../Source -I../Source/Generated -I../Source/Template -I../Demo/Include -I../Source/Generator' ;
end

%-------------------------------------------------------------------------------

dryrun = false ;
if (nargin == 1 && ~isempty (what))
    if (isequal (what, 'clean'))
        d = dir ('*.o') ;
        for k = 1:length(d)
            delete (d (k).name) ;
        end
        d = dir ('*.obj') ;
        for k = 1:length(d)
            delete (d (k).name) ;
        end
        return
    elseif (isequal (what, 'purge') || isequal (what, 'distclean'))
        gbmake ('clean') ;
        d = dir ('*.mex*') ;
        for k = 1:length(d)
            delete (d (k).name) ;
        end
        d = dir ('cover_*.c') ;
        for k = 1:length(d)
            delete (d (k).name) ;
        end
        return
    elseif (isequal (what, 'dryrun'))
        dryrun = true ;
    end
end

%-------------------------------------------------------------------------------

% Find the last modification time of any hfile.
% These are #include'd into source files.
htime = 0 ;
for k = 1:length (hfiles)
    t = datenum (hfiles (k).date) ;
    htime = max (htime, t) ;
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
    objfile = [ ofile(1:end-2) '.o' ] ;

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
    if (tc > tobj || htime > tobj)
        % compile the cfile
        fprintf ('.', cfile) ;
        % fprintf ('%s\n', cfile) ;
        mexcmd = sprintf ('mex -c %s -silent %s %s', flags, inc, cfile) ;
        if (dryrun)
            fprintf ('%s\n', mexcmd) ;
        else
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
    if (tc > tobj || any_c_compiled)
        % compile the mexFunction
        mexcmd = sprintf ('mex %s -silent %s %s %s', ...
            flags, inc, mexfunction, objlist) ;
        fprintf (':') ;
        % fprintf ('%s\n', mexfunction) ;
        if (dryrun)
            fprintf ('%s\n', mexcmd) ;
        else
            eval (mexcmd) ;
        end
    end
end

fprintf (']\n') ;

