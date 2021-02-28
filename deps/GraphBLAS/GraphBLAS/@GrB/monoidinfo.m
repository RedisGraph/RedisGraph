function monoidinfo (monoid, type)
%GRB.MONOIDINFO list the details of a GraphBLAS monoid.
%
% Usage
%
%   GrB.monoidinfo
%   GrB.monoidinfo (monoid)
%   GrB.monoidinfo (monoid, type)
%
% For GrB.monoidinfo(op), the op must be a string of the form
% 'op.type', where 'op' is listed below.  The second usage allows the
% type to be omitted from the first argument, as just 'op'.  This is
% valid for all GraphBLAS operations, since the type defaults to the
% type of the input matrices.  However, GrB.monoidinfo does not have a
% default type and thus one must be provided, either in the op as
% GrB.monoidinfo ('+.double'), or in the second argument,
% GrB.monoidinfo ('+', 'double').
%
% The MATLAB interface to GraphBLAS provides for 44 different
% monoids.  The valid monoids are: '+', '*', 'max', and 'min' for all
% but the 'logical' type, and '|', '&', 'xor', and 'eq' for the
% 'logical' type.
%
% Example:
%
%   % valid monoids
%   GrB.monoidinfo ('+.double') ;
%   GrB.monoidinfo ('*.int32') ;
%
%   % invalid monoids
%   GrB.monoidinfo ('1st.int32') ;
%   GrB.monoidinfo ('abs.double') ;
%
% See also GrB.binopinfo, GrB.descriptorinfo, % GrB.selectopinfo,
% GrB.semiringinfo, GrB.unopinfo.

% FUTURE: add complex monoids

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 0)
    help GrB.monoidinfo
elseif (nargin == 1)
    gbmonoidinfo (monoid) ;
else
    gbmonoidinfo (monoid, type) ;
end

