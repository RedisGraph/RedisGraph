function ok = spok (A)                                                      %#ok
%SPOK checks if a sparse matrix is OK
%
% Returns 1 if the sparse matrix A is OK.  Also returns 1 if A is not sparse
% (but a warning is raised, stating that the non-sparse matrix was not checked.
%
% Aborts with an error if the matrix is corrupted beyond repair.  If that
% happens, you should quit MATLAB since it's possible your workspace has also
% been corrupted.
%
% Returns 0 if the matrix has out-of-order or duplicate row indices, or
% explicit zero entries, and raises a warning.  If the matrix has out-of-order
% row indices, they can be repaired in MATLAB with A=A''.  If the matrix A has
% duplicate row indices, then A=A'' will still have duplicates, and spok(A'')
% will still issue a warning.  If the matrix has explicit zeros, you can remove
% them with A=A*1.
%
% SPOK cannot check everything.  For example, if your mexFunction has created
% a sparse matrix but written beyond the end of the array, spok may see a valid
% matrix.  However, your workspace has still been corrupted beyond repair.
%
% Example:
%
%   load west0479
%   ok = spok (west0479)         % returns 1, for a real sparse matrix
%   ok = spok (west0479 > .5)    % returns 1, for a logical sparse matrix
%   ok = spok (1i*west0479)      % returns 1, for a complex sparse matrix
%   ok = spok (speye (5))        % returns 1, for a real sparse matrix
%   ok = spok (rand (42))        % returns 1, but issues a warning (not sparse)
%
% See also sparse.

% Copyright 2008-2011, Timothy A. Davis, http://www.suitesparse.com

error ('spok mexFunction not installed') ;
