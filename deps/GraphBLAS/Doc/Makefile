#-------------------------------------------------------------------------------
# GraphBLAS/Doc/Makefile
#-------------------------------------------------------------------------------

# SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
# http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

#-------------------------------------------------------------------------------

# creates the user guide

GraphBLAS_UserGuide.pdf: GraphBLAS_UserGuide.tex GraphBLAS_UserGuide.bib \
    GraphBLAS_API_version.tex GraphBLAS_version.tex
	pdflatex GraphBLAS_UserGuide.tex
	bibtex GraphBLAS_UserGuide
	pdflatex GraphBLAS_UserGuide.tex
	pdflatex GraphBLAS_UserGuide.tex
	pdflatex GraphBLAS_UserGuide.tex

clean:
	- $(RM) -r *.out *.aux *.log *.bbl *.blg *.toc

purge: clean

distclean: clean

