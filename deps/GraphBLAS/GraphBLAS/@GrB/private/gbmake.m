function gbmake (what)
%GBMAKE compile MATLAB interface for SuiteSparse:GraphBLAS
%
% Usage:
%   gbmake
%
% gbmake compiles the MATLAB interface for SuiteSparse:GraphBLAS.  The
% GraphBLAS library must already be compiled and installed.  MATLAB 9.4
% (R2018a) or later is required.
%
% For the Mac, the GraphBLAS library must be installed in /usr/local/lib/ as
% libgraphblas.dylib.  It cannot be used where it is created in ../build,
% because of the default Mac security settings.  For Unix/Linux, the library
% used is ../build/libgraphblas.so if found, or in /usr/local/lib if not found
% there.
%
% If GraphBLAS has been initialized already, then gbmake must first finalize
% GraphBLAS, just as GrB.clear does.  It then calls GrB.init to initialize
% GraphBLAS.
%
% See also: mex, version, GrB.clear

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if verLessThan ('matlab', '9.4')
    gb_error ('MATLAB 9.4 (R2018a) or later is required') ;
end

% finish GraphBLAS
try
    GrB.finalize
catch
end

if (nargin < 1)
    what = '' ;
end

make_all = (isequal (what, 'all')) ;

% use -R2018a for the new interleaved complex API
flags = '-O -R2018a' ;

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

if (~ismac && isunix)
    flags = [ flags   ' CFLAGS="$CXXFLAGS -fopenmp -fPIC -Wno-pragmas" '] ;
    flags = [ flags ' CXXFLAGS="$CXXFLAGS -fopenmp -fPIC -Wno-pragmas" '] ;
    flags = [ flags  ' LDFLAGS="$LDFLAGS  -fopenmp -fPIC" '] ;
end

if ispc
    % Windows
    object_suffix = '.obj' ;
else
    % Linux, Mac
    object_suffix = '.o' ;
end

inc = '-Iutil -I../../../Include -I../../../Source -I../../../Source/Template' ;

ldflags = sprintf ('-L''%s/../../../build''', pwd) ;

hfiles = [ dir('*.h') ; dir('util/*.h') ] ;

cfiles = dir ('util/*.c') ;

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

    % get the full cfile filename and modification time
    cfile = [(cfiles (k).folder) filesep (cfiles (k).name)] ;
    tc = datenum (cfiles(k).date) ;

    % get the object file name
    ofile = cfiles(k).name ;
    objfile = [ ofile(1:end-2) object_suffix ] ;

    % get the object file modification time
    objlist = [ objlist ' ' objfile ] ;     %#ok
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
        fprintf ('%s\n', cfile) ;
        mexcmd = sprintf ('mex -c %s -silent %s ''%s''', flags, inc, cfile) ;
        eval (mexcmd) ;
        any_c_compiled = true ;
    end
end

mexfunctions = dir ('mexfunctions/*.c') ;

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
        mexcmd = sprintf ('mex %s -silent %s %s ''%s'' %s -lgraphblas', ...
            ldflags, flags, inc, mexfunction, objlist) ;
        fprintf ('%s\n', mexcmd) ;
        eval (mexcmd) ;
    end
end

% start GraphBLAS
try
    GrB.init
catch
end

