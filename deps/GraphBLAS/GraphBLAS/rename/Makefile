#-------------------------------------------------------------------------------
# GraphBLAS/GraphBLAS/rename/Makefile
#-------------------------------------------------------------------------------

# SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

#-------------------------------------------------------------------------------

# Linux is required, but the result (GB_rename.h) is then used for all
# platforms.  This Makefile onstructs the GB_rename.h file from the compiled
# ../../build/libgraphblas.so, which is then used to compile
# libgraphblas_matlab.so with renamed symbols, to avoid conflicting with the
# built-in libmwgraphblas.so (v3.3.3) in MATLAB R2021a and later.

# GrB_Type_new, GrB_UnaryOp_new, GrB_BinaryOp_new, GxB_SelectOp_new, and
# GxB_*Iterator* methods are #define'd as both macros and functions in
# GraphBLAS.h, so they are not renamed.  Generated functions (with "__" in
# their name) are renamed using macros internally, and so they do not need to
# be renamed with GB_rename.h.

go:
	nm -gD ../../build/libgraphblas.so | grep -v "__" > lib
	grep " GrB_" lib \
            | grep -v GrB_Type_new \
            | grep -v GrB_UnaryOp_new \
            | grep -v GrB_IndexUnaryOp_new \
            | grep -v GrB_BinaryOp_new > temp.h
	grep " GxB_" lib \
            | grep -v Iterator \
            | grep -v GxB_SelectOp_new >> temp.h
	grep " GB_"  lib \
            | grep -v Iterator >> temp.h
	awk -f rename.awk < temp.h > GB_rename.h
	rm temp.h lib
