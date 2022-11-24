SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
SPDX-License-Identifier: Apache-2.0

This folder, GraphBLAS/Source, contains all the primary source files for
GraphBLAS, and an internal include file GB.h that is meant for internal
inclusion in GraphBLAS itself.  It should not be included in end-user
applications.

The Template/ files are not compiled separately, but are #include'd into files
in this folder instead.

The Generated1/* and Generated2/* files are created by the *.m scripts from the
Generator/* files, and should not be editted.  If the Generator/ files are
modified then type:

    codegen

in Octave to construct the Generated1/* and Generated2/* files.

