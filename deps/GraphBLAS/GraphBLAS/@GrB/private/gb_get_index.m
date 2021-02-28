function [I, whole] = gb_get_index (I_input)
%GB_GET_INDEX helper function for subsref and subsasgn

whole = isequal (I_input, {':'}) & ischar (I_input {1}) ;
if (whole)
    % C (:)
    I = { } ;
elseif (iscell (I_input {1}))
    % C ({ }), C ({ list }), C ({start,fini}), or C ({start,inc,fini}).
    I = I_input {1} ;
else
    % C (I) for an explicit list I, or MATLAB colon notation
    I = I_input ;
end

