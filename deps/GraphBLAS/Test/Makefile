#-------------------------------------------------------------------------------
# GraphBLAS/Test/Makefile
#-------------------------------------------------------------------------------

# SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
# http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

#-------------------------------------------------------------------------------

# This Makefile is only intended to clean up the compiled files and files
# created by the gbmake.m function in MATLAB.  To compile GraphBLAS for use in
# MATLAB type 'gbmake' in MATLAB.  To run the tests in Test/ type 'testall'.

clean:
	$(RM) *.o *.obj

distclean: clean
	$(RM) *.mex* errlog.txt spok/*.mex* log.txt fprint.txt

purge: distclean

