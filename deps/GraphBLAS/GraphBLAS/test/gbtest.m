function gbtest
%GBTESTALL test GraphBLAS MATLAB interface
% First compile the GraphBLAS library by typing 'make' in the top-level
% GraphBLAS folder, in your system shell.  That statement will use cmake to
% compile GraphBLAS.  Use 'make JOBS=40' to compile in parallel (replace '40'
% with the number of cores in your system).  Next, go to the
% GraphBLAS/GraphBLAS/@GrB/private folder, and type the following in the MATLAB
% command window:
%
% Example:
%
%   cd GraphBLAS/GraphBLAS
%   addpath (pwd) ;
%   savepath ;          % if this fails, edit your startup.m file
%   cd @GrB/private
%   gbmake ;            % compile the MATLAB interface to GraphBLAS
%   cd ../../test
%   gbtest              % run this test
%
% NOTE: All lines of all m-files are covered by this test, but the MATLAB
% profiler shows that some lines are untested.  All of these 'untested' lines
% are end statements that appear after an error ('...') statement, so they are
% not reachable.
%
% See also GrB.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% gbtest3 requires ../demo/dnn_matlab.m and ../demo/dnn_mat2gb.m.
demo_folder = fullfile (fileparts (mfilename ('fullpath')), '../demo') ;
addpath (demo_folder) ;

gbtest0
gbtest1
gbtest2
gbtest3
gbtest4
gbtest5
gbtest6
gbtest7
gbtest8
gbtest9
gbtest10
gbtest11
gbtest12
gbtest13
gbtest14
gbtest15
gbtest16
gbtest17
gbtest18
gbtest19
gbtest20
gbtest21
gbtest22
gbtest23
gbtest24
gbtest25
gbtest26
gbtest27
gbtest28
gbtest29
gbtest30
gbtest31
gbtest32
gbtest33
gbtest34
gbtest35
gbtest36
gbtest37
gbtest38
gbtest39
gbtest40
gbtest41
gbtest42
gbtest43
gbtest44
gbtest45
gbtest46
gbtest47
gbtest48
gbtest49
gbtest50
gbtest51
gbtest52
gbtest53
gbtest54
gbtest55
gbtest56
gbtest57
gbtest58
gbtest59
gbtest60
gbtest61
gbtest62
gbtest63
gbtest64
gbtest65
gbtest66
gbtest67
gbtest68
gbtest69
gbtest70
gbtest71

gbtest99    % last test since it creates a figure

fprintf ('\ngbtest: all tests passed\n') ;

