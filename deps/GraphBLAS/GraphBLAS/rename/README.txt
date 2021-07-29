This folder contains a Makefile that extracts the external
symbols in libgraphblas.so, and creates an #include file
that renames them to a different namespace.  It is required
to install to use GraphBLAS with MATLAB R2021a and later.

To create the GB_rename.h file, first compile libgraphblas.so
and install it in /usr/local/lib/libgraphblas.so.  Then
type "make" in this folder.  This should only be required to
develop GraphBLAS since the GB_rename.h file is included in
the source distribution.

