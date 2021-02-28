function f = filetrim (filename)
% f = filetrim (filename)
%
% removes leading path and trailing "_adj.tsv.gz" from a filename

f = filename ;

i = find (f == '/', 1, 'last') ;
if (~isempty (i))
    f = f (i+1:end) ;
end

i = strfind (f, '_adj') ;
if (~isempty (i))
    f = f (1:i-1) ;
end

i = strfind (f, '.tsv') ;
if (~isempty (i))
    f = f (1:i-1) ;
end
