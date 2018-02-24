function gbcover_edit (outfile, infiles)
%GBCOVER_EDIT create a version of GraphBLAS for statement coverage tests
%
% Usage:
% gbcover_edit (outfile, infiles)
%
% Adds statement coverage counters to a set of input files, and concatenates
% them into a single output file.  The infiles argument can either be a struct
% from the 'dir' command, or it can be a string with the name of a single file.
%
% This is a very simple preprocessor that parses a subset of the C language,
% written in a specific style.  It works fine for GraphBLAS but is not
% extendable to all possible C programs (it is not intended for that use).
%
% GBCOVER_EDIT adds a statement coverage counter of the form
% "gbcov[__COUNTER__]++ ;" to specific lines of the C file.  The C preprocessor
% converts the __COUNTER__ into a compile-time constant that increments each
% time the C preprocessor sees it.
%
% A left curly bracket at the very end of a line is converted to
%
%       { gbcov[__COUNTER__]++ ;
%
% However, this breaks the left curly bracket used for global variable
% initialization, so this function does not add it to those lines.  The
% particular tests for this condition are specific to the GraphBLAS source
% code.
%
% In a switch statement, a counter is added to each case and to the default:
%
%       case:    statement
%       default: statement
%
% are converted to:
%
%       case:    gbcov[__COUNTER__]++ ; statement
%       default: gbcov[__COUNTER__]++ ; statement
%
% Finally, left curly brackets inside a function-like macro are converted:
%
%       { \
%
% becomes
%
%       { gbcov[__COUNTER__]++ ; \
%
% As a result of these rules, a single-line if statement such as:
%
%       if (expr) { statement ; }
%
% is not modified.  A general-purpose statement coverage utility would of
% course cover this case, but GraphBLAS does not have any code in that style.
% The if-statement is written instead as:
%
%       if (expr)
%       {
%           statement ;
%       }
%
% which gets properly converted to
%
%       if (expr)
%       { gbcov[__COUNTER__]++ ;
%           statement ;
%       }
%
% If you modify the C source files for GraphBLAS you can still use GBCOVER if
% you follow the same coding style.
%
% To ensure the __COUNTER__ is unique, all codes that use it must appear in a
% single C source file.  Thus, this function contatenates a set of C files into
% a single C source file.  This does not work if the files contain local static
% variables with the same names, since now they are in the same file.  This is
% a minor limitation; GraphBLAS merely ensures that its static functions have
% unique names (see for example GB_qsort_1.c).  The concatentation does not
% need to be done for *template.c files that are #include'd into other files,
% Template files will be included by the C preprocessor itself, and __COUNTER__
% will increment properly.  The counter will be unique within each template
% and with each macro expansion as well.  This is essential for testing the
% many built-in "workers" in the "switch factories" (see the GraphBLAS
% source code and look for the string "launch the switch factory").
%
% The __COUNTER__ feature is a non-standard feature that is not strictly part
% of the C language but it appears in wide range of compilers, including GNU
% gcc, clang (Mac), icc (Intel), and Microsoft Visual C.  This is sufficient
% since the statement coverage tests are not part of the C-callable GraphBLAS
% library itself.  They are only used here in this test, and only through
% MATLAB.  Statement coverage tests are not required to compile and use
% GraphBLAS.  They are not needed to run the GraphBLAS tests in
% GraphBLAS/Test, either.  They will just be run without statement coverage
% counts.
%
% This function is not meant to typed in at the command line.  See gbcover
% instead.
%
% See also: gbcover

%  SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
%  http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

f_output = fopen (outfile, 'w') ;

% infiles can be a struct from dir, or a single string with one filename
if (~isstruct (infiles))
    infiles = struct ('folder', '.', 'name', infiles) ;
end
nfiles = length (infiles) ;

for k = 1:nfiles

    infile = [infiles(k).folder filesep infiles(k).name] ;

    f_input  = fopen (infile,  'r') ;

    % A left curly bracket at the end of a line gets changed to "{ gbcov[n]++;"
    % but some brackets can't be modified this way, such as the curly bracket
    % in a switch statement or in an initializer for a global variable
    % (GrB_error.c and GrB_ops.c).  The following flag keeps track of this
    % condition:
    prev_line_was_switch_or_struct = false ;

    % get the first line
    cline = fgetl (f_input) ;

    while (ischar (cline))

        if (isempty (cline))

            % empty line: as-is
            fprintf (f_output, '\n') ;

        elseif (cline (end) == '{')

            % left curly brackect at the end of the line
            % "{" changes to "{ gbcov[n]++ ; "
            % but not for the line just after a switch statement or initializer

            if (prev_line_was_switch_or_struct || ...
                ~isempty (strfind (cline, '=')))
                % do nothing in this case
                fprintf (f_output, '%s\n', cline) ;
            else
                fprintf (f_output, '%s gbcov[__COUNTER__]++ ;\n', cline) ;
            end

        elseif (~isempty (strfind (cline, ' case ')) || ...
                ~isempty (strfind (cline, ' default ')))

            % a switch case statement, or "default:"
            % "case stuff: statement" => "case stuff : gbcov[n]++ ; statement"
            colon = find (cline == ':', 1) ;
            if (~isempty (colon))
                fprintf (f_output, '%s : gbcov[__COUNTER__]++ ; %s\n', ...
                    cline (1:colon-1), cline (colon+1:end)) ;
            else
                % do nothing in this case
                fprintf (f_output, '%s\n', cline) ;
            end

        elseif (isequal (regexp (cline, '\s*\{\s*\\'), 1))

            % coverage statement inside a macro, "{ \" => "{ gbcov[n]++ ; \"

            if (prev_line_was_switch_or_struct)
                % do nothing in this case
                fprintf (f_output, '%s\n', cline) ;
            else
                fprintf (f_output, '{ gbcov[__COUNTER__]++ ; \\\n') ;
            end

        elseif (isequal (regexp (cline, ' \s*\#include'), 1))

            % change #include "file.c" -> #include "cover_file.c"
            s = find (cline == '"', 1) ;
            fprintf (f_output, '%scover_%s\n', cline (1:s), cline (s+1:end)) ;

        else

            % otherwise the line is copied as-is
            fprintf (f_output, '%s\n', cline) ;

        end

        % check if it was a switch or struct statement.  This is admittedly
        % a bit of a hack, particularly the third case (see GrB_ops.c).
        % Except for the third case below, the conversion implemented by
        % this function is fairly robust.
        prev_line_was_switch_or_struct = ...
            ~isempty (strfind (cline, 'switch (')) || ...
            ~isempty (strfind (cline, 'struct')) || ...
            ~isempty (strfind (cline, '_opaque')) ;

        % get the next line
        cline = fgetl (f_input) ;

    end

    fclose (f_input) ;
end

fclose (f_output) ;

