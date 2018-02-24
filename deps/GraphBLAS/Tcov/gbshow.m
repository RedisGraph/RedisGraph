function gbshow
%GBSHOW create a test coverage report in cover_gb_report.c

f_input = fopen ('cover_gb_exp.c', 'r') ;
f_output = fopen ('cover_gb_report.c', 'w') ;

load gbstat.mat

% get the first line
cline = fgetl (f_input) ;

kfound = 0 ;

while (ischar (cline))

    if (~isempty (strfind (cline, 'gbcov[')) && ...
        ~isempty (strfind (cline, '++')))
        % got one; get the count
        k1 = strfind (cline, '[') ;
        k2 = strfind (cline, ']') ;
        s = cline (k1+1:k2-1) ;
        i = str2num (s) + 1 ;
        c = GraphBLAS_gbcov (i) ;
        kfound = kfound + 1 ;
        assert (i == kfound) ;
        if (c == 0)
            fprintf (f_output, 'NOT COVERED (%d):\n', i) ;
        else
            fprintf (f_output, 'Covered (%d): %d\n', i, c) ;
        end
    else
        fprintf (f_output, '%s\n', cline) ;
    end

    cline = fgetl (f_input) ;
end

fclose (f_output) ;
fclose (f_input) ;

