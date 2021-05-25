function S = struct (G)
%GRB.STRUCT return the opaque (private) contents of a @GrB object.
% S = struct (G) returns the opaque properties of a @GrB object as a
% struct.  The properties of G are private, and thus the fields of S
% cannot be interpretted except by GrB (S).  However, S can be can be
% saved to a file and then loaded back in, which may facilitate
% portability with future SuiteSparse:GraphBLAS versions, in case the
% contents of a @GrB object changes.  S can be converted back into a @GrB
% object with G = GrB (S).
%
% See also load, save, properties, GrB.version, GrB.ver.

% Note for Octave users:  Octave cannot save or load a @GrB object G
% to/from a file, but it should be able to save/load the struct S.  When S
% is loaded back in from the file, it can be converted to a @GrB object
% with G = GrB (S).  A struct S constructed from a @GrB object G via S =
% struct (G) always has the fieldname 'GraphBLASv4' as its first field, in
% SuiteSparse:GraphBLAS version v4.0.1.  If the format of the struct
% changes in the future, another field will be used, unique to that
% version of SuiteSparse:GraphBLAS, and a backward-compatibility process
% will be written so that G = GrB (S) converts S into a @GrB object for
% the then-current version of SuiteSparse:GraphBLAS.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

S = G.opaque ;
