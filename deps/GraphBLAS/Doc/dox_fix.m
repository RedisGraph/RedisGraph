function s = dox_fix (s)
%DOX_FIX add escape characters to a string, for Doxygen
%
% s = dox_fix (s)

s = strrep (s, '<', '\<') ;
s = strrep (s, '>', '\>') ;
s = strrep (s, '#', '\#') ;
s = strrep (s, '&', '\&') ;
s = strrep (s, '"', '\"') ;
