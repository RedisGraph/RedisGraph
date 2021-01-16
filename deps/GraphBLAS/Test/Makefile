#-------------------------------------------------------------------------------
# GraphBLAS/Test/Makefile
#-------------------------------------------------------------------------------

# SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

#-------------------------------------------------------------------------------

# This Makefile is only intended to clean up the compiled files and files
# created by the make.m function in MATLAB.  To compile GraphBLAS for use in
# MATLAB type 'make' in MATLAB.  To run the tests in Test/ type 'testall'.

GraphBLAS_Test.pdf: GraphBLAS_Test.tex 
	pdflatex GraphBLAS_Test.tex
	pdflatex GraphBLAS_Test.tex

clean:
	- $(RM) -r *.o *.obj *.out *.aux *.log *.bbl *.blg *.toc

distclean: clean
	$(RM) *.mex* errlog*.txt spok/*.mex* log.txt fprint.txt

purge: distclean

