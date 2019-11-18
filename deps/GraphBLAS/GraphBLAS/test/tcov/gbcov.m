function gbcov
%GBCOV run all GraphBLAS tests, with statement coverage

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% compile the coverage-test version of the @GrB mexFunctions
clear all
global gbcov_global

gbcovmake
addpath ('..') ;            % add the test folder to the path
rmpath ('../..') ;          % remove the regular @GrB class
addpath ('tmp') ;           % add the modified @GrB class

% run the tests
gbtest ;

addpath ('../..') ;         % add back the regular @GrB class
rmpath ('tmp') ;            % remove the modified @GrB class

% report the coverage
gbcovshow ;

