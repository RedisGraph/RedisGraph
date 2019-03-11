SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved
http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

This folder, GraphBLAS/Source, contains all the primary source files
for GraphBLAS, and an internal include file GB.h that is meant for
internal inclusion in GraphBLAS itself.  It should not be included in
end-user applications.

The Template/ files are not compiled separately, but are #include'd
into files in this folder instead.

The Generated/ files are created by the axb*m scripts from Generator/GB_AxB.*
and should not be editted.  If the Generator/ files are modified then do:

    axb

in MATLAB to construct the Generated/ files.

