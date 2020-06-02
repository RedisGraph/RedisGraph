% GraphBLAS/GraphBLAS/test/tcov/Contents.m:
%
% The gbcov script compiles the MATLAB interface with statement coverage
% enabled, and then runs the full test suite (../gbtest).  Next, it uses
% gbcovshow to create the statement coverage report in tmp/cover.  To remove
% all temporary files, use 'make distclean' or remove the tmp/* files and
% folders.
%
% To run these tests, if GraphBLAS/@GrB is initialized, first use
% GrB.finalize.  Then gbcov can load the modified MATLAB interface.
%
%   gbcov      - run all GraphBLAS tests, with statement coverage
%
% Utilities:
%
%   gbcovmake  - compile the MATLAB interface for statement coverage testing
%   gbcovshow  - report GraphBLAS statement coverage
%   gbcov_edit - create a version of GraphBLAS for statement coverage tests

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

