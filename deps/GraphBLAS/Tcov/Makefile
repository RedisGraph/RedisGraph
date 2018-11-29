#-------------------------------------------------------------------------------
# GraphBLAS/Tcov/Makefile
#-------------------------------------------------------------------------------

#  SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
#  http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

#-------------------------------------------------------------------------------

# This Makefile is only intended to clean up the compiled files and files
# created by the gbcover.m function in MATLAB.  To compile GraphBLAS for
# statement coverage testing, type 'gbcover' in MATLAB.  To compile the tests
# and run them, type 'testcov' in MATLAB.

clean:
	$(RM) *.o *.obj

distclean: clean
	$(RM) *.mex*
	$(RM) tmp*/* errlog.txt gbstat.mat fprint.txt

purge: distclean
	$(RM) log.txt

