function descriptorinfo (d)
%GRB.DESCRIPTOR list the contents of a SuiteSparse:GraphBLAS descriptor.
%
% Usage:
%
%   GrB.descriptorinfo
%   GrB.descriptorinfo (d)
%
% The GraphBLAS descriptor is a MATLAB struct that can be used to modify
% the behavior of GraphBLAS operations.  It contains the following
% components, each of which are a string or a number.  Any component of
% struct that is not present is set to the default value.  If the
% descriptor d is empty, or not present, in a GraphBLAS function, all
% default settings are used.
%
% The following descriptor values are strings:
%
%   d.out   'default' or 'replace'      determines if C is cleared before
%                                         the accum/mask step
%
%   d.mask  'default'       use M as the mask (if present)
%           'complement'    use ~M as the mask
%           'structural' or 'structure'   use the pattern of M, not its values
%           'structural complement'       use the pattern of ~M
%
%   d.in0   'default' or 'transpose'    determines A or A' is used
%   d.in1   'default' or 'transpose'    determines B or B' is used
%
%   d.axb   'default', 'Gustavson', 'heap', 'hash', or 'dot'.  Determines the
%            method used in GrB.mxm.  The default is to let GraphBLAS
%            determine the method automatically, via a heuristic.
%
%   d.kind   For most GrB.methods, this is a string equal to 'default',
%            'GrB', 'sparse', or 'full'.  The default is d.kind = 'GrB',
%            where the GraphBLAS operation returns an object, which is
%            preferred since GraphBLAS sparse matrices are faster and can
%            represent many more data types.  However, if you want a
%            standard MATLAB sparse matrix, use d.kind='sparse'.  Use
%            d.kind='full' for a MATLAB dense matrix.  For any GrB.method
%            that takes a descriptor, the following uses are the same,
%            but the first method is faster and takes less temporary
%            workspace:
%
%               d.kind = 'sparse' ;
%               S = GrB.method (..., d) ;
%
%               % with no d, or d.kind = 'default'
%               S = double (GrB.method (...)) :
%
%   d.base  A string equal to 'default', 'zero-based', 'one-based',
%           or 'one-based int'.  The default is 'one-based'.  If d.base is
%           'zero-based', then indices are zero-based, in the range 0 to n-1,
%           for a matrix of dimension n.
%
%   d.format a string, either 'by row' or 'by col', which defines the
%           format of the GraphBLAS output matrix C.  The following rules
%           are used to determine the format of the result, in order:
%
%           (1) If the format is determined by the descriptor to the
%               method, then that determines the format of C.
%           (2) If C is a column vector then C is stored by column.
%           (3) If C is a row vector then C is stored by row.
%           (4) If the method has a first matrix input (usually called A),
%               and it is not a row or column vector, then its format is
%               used for C.
%           (5) If the method has a second matrix input (usually called
%               B), and it is not a row or column vector, then its format
%               is used for C.
%           (6) Otherwise, the global default format is used for C.
%               See GrB.format for details.
%
% These descriptor values are scalars:
%
%   d.nthreads  max # of threads to use; default is omp_get_max_threads.
%   d.chunk     controls # of threads to use for small problems.
%
% GrB.descriptorinfo (d) lists the contents of a GraphBLAS descriptor and
% checks if its contents are valid.  Also refer to the
% SuiteSparse:GraphBLAS User Guide for more details.
%
% See also GrB.binopinfo, GrB.monoidinfo, GrB.selectopinfo,
% GrB.semiringinfo, GrB.unopinfo.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 0)
    help GrB.descriptorinfo
    gbdescriptorinfo ;
else
    gbdescriptorinfo (d) ;
end

