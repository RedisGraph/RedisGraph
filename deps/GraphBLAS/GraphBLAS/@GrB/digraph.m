function DiGraph = digraph (G, option)
%GRAPH convert a GraphBLAS matrix into a MATLAB directed DiGraph.
% DiGraph = digraph (G) converts a GraphBLAS matrix G into a directed MATLAB
% DiGraph.  G must be square.  If G is logical, then no weights are added to
% the DiGraph.  If G is single or double, these become the weights of the
% MATLAB DiGraph.  If G is integer, the DiGraph is constructed with weights of
% type double.  An optional string argument can appear after G:
%
%   DiGraph = digraph (G, 'omitselfloops') ignores the diagonal of G, and the
%   resulting MATLAB DiGraph has no self-edges.  The default is that self-edges
%   are created from any diagonal entries of G.
%
% Example:
%
%   G = GrB (sprand (8, 8, 0.2))
%   DiGraph = digraph (G)
%   h = plot (DiGraph) ;
%   h.NodeFontSize = 20 ;
%   h.ArrowSize = 20 ;
%   h.LineWidth = 2 ;
%   h.EdgeColor = [0 0 1] ;
%   t = title ('random directed graph with 8 nodes') ;
%   t.FontSize = 20 ;
%
% See also graph, digraph, GrB/graph, plot.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

type = GrB.type (G) ;
[m, n] = size (G) ;
if (m ~= n)
    gb_error ('G must be square') ;
end

% get the string options
omitself = false ;
if (nargin > 1)
    if (isequal (lower (option), 'omitselfloops'))
        omitself = true ;
    else
        gb_error ('unknown option') ;
    end
end

% apply the options
if (omitself)
    % ignore diagonal entries of G
    G = GrB.offdiag (G) ;
end

% construct the digraph
if (isequal (type, 'logical'))
    DiGraph = digraph (logical (G)) ;
elseif (isequal (type, 'double'))
    DiGraph = digraph (double (G)) ;
elseif (isequal (type, 'single'))
    [i, j, x] = GrB.extracttuples (G) ;
    DiGraph = digraph (i, j, x, n) ;
else
    % all other types (int* and uint*) must be cast to double
    DiGraph = digraph (double (GrB (G, 'double'))) ;
end

