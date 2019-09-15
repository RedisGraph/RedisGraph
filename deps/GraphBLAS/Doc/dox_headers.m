%DOX_HEADERS: run this script in MATLAB before running 'make dox'
% This script creates Doc/headers_for_doxygen.h, which contains
% comment blocks for each file that are then parsed by doxygen.

clear
ff = fopen ('headers_for_doxygen.h', 'w') ;

fprintf (ff, '/** \\file headers_for_doxygen.h\n') ;
fprintf (ff, '\\brief auto-generated headers for doxygen\n') ; 
fprintf (ff, 'constructed by dox_headers.m\n') ; 
fprintf (ff,'*/\n') ;

d = [dir('../Source')
     dir('../Include')
     dir('../Source/Template')
     dir('../Source/Generator')] ;

for k = 1:length(d)
    folder = d(k).folder ;
    file = d(k).name ;
    len = length (file) ;
    if (len < 2)
        continue ;
    end
    fin = file ((len-1):len) ;

    if (isequal (file, 'GB_AxB.h'))
        fprintf (ff,'\n\n/** \\file %s\n', file) ;
        fprintf (ff,'\\brief Source/Generator/GB_AxB.h: used to create GB_AxB__include.h\n') ;
        fprintf (ff,'*/\n') ;

    elseif (isequal (file, 'GB_binop.h'))
        fprintf (ff,'\n\n/** \\file %s\n', file) ;
        fprintf (ff,'\\brief Source/Generator/GB_binop.h: used to create GB_binop__include.h\n') ;
        fprintf (ff,'*/\n') ;

    elseif (isequal (file, 'GB_red.h'))
        fprintf (ff,'\n\n/** \\file %s\n', file) ;
        fprintf (ff,'\\brief Source/Generator/GB_red.h: used to create GB_red__include.h\n') ;
        fprintf (ff,'*/\n') ;

    elseif (isequal (file, 'GB_sel.h'))
        fprintf (ff,'\n\n/** \\file %s\n', file) ;
        fprintf (ff,'\\brief Source/Generator/GB_sel.h: used to create GB_sel__include.h\n') ;
        fprintf (ff,'*/\n') ;

    elseif (isequal (file, 'GB_unaryop.h'))
        fprintf (ff,'\n\n/** \\file %s\n', file) ;
        fprintf (ff,'\\brief Source/Generator/GB_unaryop.h: used to create GB_unaryop__include.h\n') ;
        fprintf (ff,'*/\n') ;

    elseif (isequal (fin, '.c') || isequal (fin, '.h'))

        fprintf (ff,'\n\n/** \\file %s\n', file) ;
        f = fopen ([folder '/' file], 'r') ;
        line = fgets (f) ;
        line = fgets (f) ;
        len = length (line) ;
        if (len < 2)
            continue ;
        end
        need_par = false ;

        fprintf (ff, '\\brief %s\n', dox_fix (line (3:end))) ;
        % fprintf (ff, '\\verbatim\n') ;

        while (1)
            line = fgets (f) ;
            if (~ischar (line))
                break ;
            end
            len = length (line) ;
            c = '' ;
            if (len > 0)
                c = line (1) ;
            end
            if (c == '{' || c == '#' || c == ' ')
                % fprintf (ff,'break %s\n', line) ;
                break ;
            elseif (len <= 2)
                need_par = true ;
            elseif (line (3) == '-') ;
                continue ;
            elseif (isequal (line (1:2), '//'))
                if (~isempty (strfind (line, 'Davis')))
                    continue ;
                end
                if (~isempty (strfind (line, 'License')))
                    continue ;
                end
                if (need_par)
                    fprintf (ff,'\\par\n') ;
                    need_par = false ;
                end
                fprintf (ff,'%s', dox_fix (line (3:end))) ;
            else
                % fprintf (ff,'break %s\n', line) ;
                break ;
            end
        end

        % fprintf (ff,'\\endverbatim\n*/\n') ;
        fprintf (ff,'*/\n') ;
        fclose (f) ;
    end
end

fclose (ff) ;
