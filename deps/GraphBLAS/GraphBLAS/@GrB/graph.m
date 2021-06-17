function Graph = graph (G, varargin)
%GRAPH convert a GraphBLAS matrix into a MATLAB undirected Graph.
% Graph = graph (G) converts a GraphBLAS matrix G into an undirected MATLAB
% Graph.  G is assumed to be symmetric; only tril (G) is used by default.
% G must be square.  If G is logical, then no weights are added to the
% Graph.  If G is single or double, these become the weights of the MATLAB
% Graph.  If G is integer, the Graph is constructed with weights of type
% double.
%
% Graph = graph (G, ..., 'upper') uses triu (G) to construct the Graph.
% Graph = graph (G, ..., 'lower') uses tril (G) to construct the Graph.
% The default is 'lower'.
%
% Graph = graph (G, ..., 'omitselfloops') ignores the diagonal of G, and
% the resulting MATLAB Graph has no self-edges.  The default is that
% self-edges are created from any diagonal entries of G.
%
% Example:
%
%   G = GrB (bucky) ;
%   Graph = graph (G)
%   plot (Graph)
%
% See also graph, digraph, GrB/digraph.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

G = G.opaque ;

[m, n, type] = gbsize (G) ;
if (m ~= n)
    error ('G must be square') ;
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
            error ('unknown option') ;
    end
end

% apply the options
if (omitself)
    % ignore diagonal entries of G
    if (isequal (side, 'upper'))
        G = gbselect ('triu', G, 1) ;
    elseif (isequal (side, 'lower'))
        G = gbselect ('tril', G, -1) ;
    end
else
    % include diagonal entries of G
    if (isequal (side, 'upper'))
        G = gbselect ('triu', G, 0) ;
    elseif (isequal (side, 'lower'))
        G = gbselect ('tril', G, 0) ;
    end
end

% construct the graph
switch (type)

    case { 'single' }

        % The MATLAB graph(...) function can accept x as single, but not
        % from a MATLAB sparse matrix.  So extract the tuples of G first.
        [i, j, x] = gbextracttuples (G) ;
        Graph = graph (i, j, x, n) ;

    case { 'logical' }

        % The MATLAB digraph(...) function allows for logical
        % adjacency matrices (no edge weights are created).
        Graph = graph (gbmatlab (G, 'logical'), side) ;

    otherwise

        % typecast to double
        Graph = graph (gbmatlab (G, 'double'), side) ;
end

