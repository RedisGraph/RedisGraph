function k = gb_convert_index_2d_to_1d (i, j, m)
%GB_CONVERT_INDEX_2D_TO_1D convert 2D indices to 1D
% the indices must be zero-based

k = i + j * m ;

