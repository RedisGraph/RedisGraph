SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
SPDX-License-Identifier: Apache-2.0

MKL integration is work in progress; these files are currently not used in the
production version.  To use them, move or copy them into the ../Source
directory (put *template.c files in ../Source/Template), uncomment all
"#include "GB ...mkl_template..." lines, enable MKL in the CMakeLists.txt file,
and edit Config/GraphBLAS.h.in to include (not #include) the contents of
for_GraphBLAS.h.in.

