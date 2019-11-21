function [args, is_GrB] = gb_get_args (varargin)
%GB_GET_ARGS get arguments for a GraphBLAS method.
% Get the arguments and the descriptor for a GrB.method.  Any input
% arguments that are GraphBLAS sparse matrix objects are replaced with the
% struct arg.opaque so that they can be passed to the underlying
% mexFunction.  Next, the descriptor is modified to change the default
% d.kind.
%
% All mexFunctions in private/mexFunction/*.c require the descriptor to be
% present as the last argument.  They are not required for the user-
% accessible GrB.methods.  If the descriptor d is not present, then it is
% created and appended to the argument list, with d.kind = 'GrB'.  If the
% descriptor is present and d.kind does not appear, then d.kind = 'GrB' is
% set.  Finally, is_GrB is set true if d.kind is 'GrB'.  If d.kind is 'GrB',
% then the underlying mexFunction returns a GraphBLAS struct, which is then
% converted above to a GraphBLAS object.

% get the args and extract any GraphBLAS matrix structs
args = varargin ;
for k = 1:length (args)
    if (isa (args {k}, 'GrB'))
        args {k} = args {k}.opaque ;
    end
end

% find the descriptor
is_GrB = true ;
if (~isempty (args))
    % get the last input argument and see if it is a GraphBLAS descriptor
    d = args {end} ;
    if (isstruct (d) && ~isfield (d, 'GraphBLAS'))
        % found the descriptor.  If it does not have d.kind, add it.
        if (~isfield (d, 'kind'))
            d.kind = 'GrB' ;
            args {end} = d ;
            is_GrB = true ;
        else
            is_GrB = isequal (d.kind, 'GrB') || isequal (d.kind, 'default') ;
        end
    else
        % the descriptor is not present; add it
        args {end+1} = struct ('kind', 'GrB') ;
        is_GrB = true ;
    end
end

