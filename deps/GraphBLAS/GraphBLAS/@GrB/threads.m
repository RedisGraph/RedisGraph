function nthreads = threads (varargin)
%GRB.THREADS get/set the max number of threads to use in GraphBLAS.
%
% Usage:
%   nthreads = GrB.threads ;      % get the current maximum # of threads
%   GrB.threads (nthreads) ;      % set the maximum # of threads
%
% GrB.threads gets and/or sets the maximum number of threads to use in
% GraphBLAS.  By default, if GraphBLAS has been compiled with OpenMP, it
% uses the number of threads returned by omp_get_max_threads.  Otherwise,
% it can only use a single thread.
%
% Changing the number of threads with GrB.threads(nthreads) causes all
% subsequent GraphBLAS operations to use at most the given number of
% threads.  GraphBLAS may use fewer threads, if the problem is small (see
% GrB.chunk).  The setting is kept for the remainder of the current MATLAB
% session, or until 'clear all' or GrB.clear is used, at which point the
% setting reverts to the default number of threads.
%
% MATLAB can detect the number of physical and logical cores via an
% undocumented builtin function: ncores = feature('numcores'), or via
% maxNumCompThreads.
%
% Example:
%
%   feature ('numcores') ;          % print info about cores
%   ncores = feature ('numcores') ; % get # of logical cores MATLAB uses
%   ncores = maxNumCompThreads ;    % same as feature ('numcores')
%   GrB.threads (2*ncores) ;         % GraphBLAS will use at most 2*ncores
%                                   % threads
%
% See also feature, maxNumCompThreads, GrB.chunk.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

nthreads = gbthreads (varargin {:}) ;

