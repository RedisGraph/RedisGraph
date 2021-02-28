function [nthreads chunk] = nthreads_get
%NTHREADS_GET get # of threads and chunk to use in GraphBLAS
%
% [nthreads chunk] = nthreads_get

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

global GraphBLAS_nthreads
if (isempty (GraphBLAS_nthreads))
    nthreads_set (1) ;
end
nthreads = GraphBLAS_nthreads ;

if (nargout > 1)
    global GraphBLAS_chunk
    if (isempty (GraphBLAS_chunk))
        [nthreads chunk] = nthreads_set (nthreads, 4096) ;
    end
    chunk = GraphBLAS_chunk ;
end

