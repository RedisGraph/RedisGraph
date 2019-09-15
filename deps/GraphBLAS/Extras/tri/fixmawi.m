function filefix = fixmawi (filename)
i = strfind (filename, '.v') ;

if (~isempty (i))
    filefix = ['mawi_' filename(1:i-1)] ;
else
    filefix = filename ;
end
