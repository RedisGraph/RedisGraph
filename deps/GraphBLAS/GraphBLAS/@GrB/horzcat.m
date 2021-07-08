function C = horzcat (varargin)
%HORZCAT horizontal concatenation.
% [A B] or [A,B] is the horizontal concatenation of A and B.
% Multiple matrices may be concatenated, as [A, B, C, ...].
% If the matrices have different types, the type is determined
% according to the rules in GrB.optype.
%
% See also GrB/vertcat, GrB/cat, GrB.cell2mat, GrB/mat2cell, GrB/num2cell.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

% get the input matrices
nmatrices = length (varargin) ;
for k = 1:nmatrices
    Tile = varargin {k} ;
    if (isobject (Tile))
        varargin {k} = Tile.opaque ;
    end
end

% concatenate the matrices
C = GrB (gbcat (varargin)) ;

