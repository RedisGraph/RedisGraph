SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

GraphBLAS/Tcov: statement coverage tests

Since nearly all GraphBLAS tests are in MATLAB, I have taken the unusual step
of creating a statement coverage mechanism to use within a MATLAB mexFunction.
To compile GraphBLAS for statement coverage testing, and to run the tests, do:

    testcov

Statement coverage tests results will be saved in log.txt.  The entire test
can take about 20 minutes, including the compile time.

To list the lines covered by the test, do this in MATLAB:

    gbshow

To remove all compiled files, type this in the Unix/Linux shell:

    make distclean

Or, delete these files manually:

    *.o *.obj *.mex* cover_*.c errlog.txt gbstat.mat

To also remove the log.txt file:

    make purge

--------------------------------------------------------------------------------
Files in GraphBLAS/Tcov:
--------------------------------------------------------------------------------

    Contents.m     for 'help Tcov' in MATLAB; list of files

    gbcover.m      compile GraphBLAS for statement coverage testing
    gbcover_edit.m create a version of GraphBLAS for statement coverage tests
    testcov.m      run all GraphBLAS tests, with statement coverage
    gbshow.m       create a test coverage report in cover_gb_report.c
    Makefile       just for 'make clean' and 'make purge'
    README.txt     this file

    gbcover.h           test coverage include file
    gbcover_finish.c    save the last test coverage counter
    gbcover_start.c     declare the test coverage counter array
    gbcover_util.c      get/put the coverage to/from MATLAB
    log_*.txt           100% test coverage certificate

