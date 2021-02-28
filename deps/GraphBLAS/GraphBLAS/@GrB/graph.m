function Graph = graph (G, varargin)
%GRAPH convert a GraphBLAS matrix into a MATLAB undirected Graph.
% Graph = graph (G) converts a GraphBLAS matrix G into an undirected MATLAB
% Graph.  G is assumed to be symmetric; only tril (G) is used by default.  G
% must be square.  If G is logical, then no weights are added to the Graph.  If
% G is single or double, these become the weights of the MATLAB Graph.  If G is
% integer, the Graph is constructed with weights of type double.  Optional
% string arguments can appear after G, in any order:
%
%   Graph = graph (G, ..., 'upper') uses only triu (G) to construct the Graph.
%   Graph = graph (G, ..., 'lower') uses only tril (G) to construct the Graph.
%   The default is 'lower'.
%
%   Graph = graph (G, ..., 'omitselfloops') ignores the diagonal of G, and the
%   resulting MATLAB Graph has no self-edges.  The default is that self-edges
%   are created from any diagonal entries of G.
%
% Example:
%
%   G = GrB (bucky) ;
%   Graph = graph (G)
%   plot (Graph)
%
% See also graph, digraph, GrB/digraph.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

type = GrB.type (G) ;
[m, n] = size (G) ;
if (m ~= n)
    gb_error ('G must be square') ;
end

% get the string options
side = 'lower' ;
omitself = false ;
for k = 1:nargin-1
    arg = lower (varargin {k}) ;
    switch arg
        case { 'upper', 'lower' }
            side = arg ;
        case { 'omitselfloops' }
            omitself = true ;
        otherwise
            gb_error ('unknown option') ;
    end
end

% apply the options
if (omitself)
    % ignore diagonal entries of G
    if (isequal (side, 'upper'))
        G = triu (G, 1) ;
    elseif (isequal (side, 'lower'))
        G = tril (G, -1) ;
    end
else
    % include diagonal entries of G
    if (isequal (side, 'upper'))
        G = triu (G) ;
    elseif (isequal (side, 'lower'))
        G = tril (G) ;
    end
end

% construct the graph
if (isequal (type, 'logical'))
    Graph = graph (logical (G), side) ;
elseif (isequal (type, 'double'))
    Graph = graph (double (G), side) ;
elseif (isequal (type, 'single'))
    [i, j, x] = GrB.extracttuples (G) ;
    Graph = graph (i, j, x, n) ;
else
    % all other types (int* and uint*) must be cast to double
    Graph = graph (double (GrB (G, 'double')), side) ;
end

