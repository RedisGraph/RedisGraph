function init
%GRB.INIT initialize SuiteSparse:GraphBLAS.
%
% Usage:
%
%   GrB.init
%
% GrB.init initializes all SuiteSparse:GraphBLAS settings to their
% defaults.  In prior versions (v3.1.2), its use was required before
% calling any SuiteSparse:GraphBLAS function in MATLAB.  Its use is now
% optional in this version of SuiteSparse:GraphBLAS.
%
% See also: GrB.clear, GrB.finalize, startup.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

gbsetup ;

