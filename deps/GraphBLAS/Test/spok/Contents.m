% SPOK : checks the validity of a built-in sparse matrix.
%
% This function is of little use for those using purely M-files.  It is
% extremely useful for those who write mexFunctions that return sparse
% matrices.  A built-in sparse matrix is stored in compressed-column form,
% where each column is stored as a list of entries with their row indices and
% their corresponding numerical values.  Row indices must appear in ascending
% order, and no explicitly zero numerical entries can appear.  Constructing a
% valid sparse matrix in a mexFunction can be difficult.  This function will
% help you to know if you've done that correctly.
%
% Files
%   spok         - checks if a sparse matrix is OK
%   spok_install - compiles and installs the SPOK mexFunction
%   spok_test    - installs and tests SPOK
%
% Example
%
%   load west0479
%   A = west0479 ;
%   spok (A)                % double sparse
%   spok (A + 1i*A)         % complex sparse
%   spok (A > .5)           % logical sparse

% Copyright 2008-2011, Timothy A. Davis, http://suitesparse.com
% SPDX-License-Identifier: Apache-2.0

