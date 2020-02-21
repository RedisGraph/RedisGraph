# GraphBLAS/GraphBLAS: MATLAB interface for SuiteSparse:GraphBLAS

SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

The GrB class provides an easy-to-use MATLAB interface to SuiteSparse:GraphBLAS.

To install it for use in MATLAB, first compile the GraphBLAS library,
-lgraphblas.  See the instructions in the top-level GraphBLAS folder for
details.  Be sure to use OpenMP for best performance.  The default installation
process places the GraphBLAS library in /usr/local/lib.  If you do not have
root access and cannot install GraphBLAS into /usr/local/lib, then follow the
instructions below to modify your library path, but instead of /usr/local/lib,
use /home/me/SuiteSparse/GraphBLAS/build, where
"/home/me/SuiteSparse/GraphBLAS" is where you placed your copy of GraphBLAS.

MATLAB needs to know where to find the compiled GraphBLAS library.  On
Linux/Unix, if you are using the bash or korn shells, make sure that add the
following to your login profile (typically .bash_profile for bash, or .profile
for korn):

    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
    export LD_LIBRARY_PATH

On Linux/Unix with the csh, tcsh or related shells, use:

    setenv PATH $PATH\:/usr/local/lib

On the Mac, use the following:

    DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/usr/local/lib
    export DYLD_LIBRARY_PATH

If you modify your library path, above, you need to execute those commands as
well, before you start MATLAB.

For more details on setting your Linux/Unix/Mac library path for MATLAB see
https://www.mathworks.com/help/matlab/matlab_external/building-on-unix-operating-systems.html

Next, start MATLAB and go to this GraphBLAS/GraphBLAS folder.  Type

    addpath (pwd)

to add the GraphBLAS interface to your path.  Then do

    savepath

Or, if that function is not allowed because of file permissions, add this
command to your startup.m file:

    % add the MATLAB interface to the MATLAB path
    addpath ('/home/me/SuiteSparse/GraphBLAS/GraphBLAS') :

where the path /home/me/SuiteSparse/GraphBLAS/GraphBLAS is the full path to
this folder.

The name "GraphBLAS/GraphBLAS" is used for this folder so that this can be done
in MATLAB:

    help graphblas

To get additional help, type:

    methods GrB
    help GrB

Next, go to the GraphBLAS/GraphBLAS/@GrB/private folder and compile the MATLAB
mexFunctions.  Assuming your working directory is GraphBLAS/GraphBLAS
(where this README.md file is located), do the following:

    cd @GrB/private
    gbmake

To run the demos, go to the GraphBLAS/GraphBLAS/demo folder and type:

    gbdemo
    gbdemo2

The output of these demos on a Dell XPS 13 laptop and an NVIDIA DGX Station can
also be found in GraphBLAS/GraphBLAS/demo/html, in both PDF and HTML formats.

To test your installation, go to GraphBLAS/GraphBLAS/test and type:

    gbtest

If everything is successful, it should report 'gbtest: all tests passed'.
Note that gbtest tests all features of the MATLAB interface to
SuiteSparse/GraphBLAS, including error handling, so you can expect to see
error messages during the test.  This is expected.

# FUTURE: Not yet supported for GrB matrices in MATLAB:

    linear indexing
    complex matrices
    2nd output for [x,i] = max (...) and [x,i] = min (...); needs
        modified reduction methods inside GraphBLAS
    'includenan' for min and max
    singleton expansion
    3D and higher dimensional matrices:
        this might be done by converting the higher dimensioal
        indices down to a large 2D space, ad relying on hypersparsity.
    saturating element-wise binary and unary operators for integers 

The last two features don't exist for MATLAB sparse matrices.

For Windows: Microsoft Visual Studio does not support OpenMP tasking,
    which means that the internal sort is not parallel, but sequential.
    This affects GrB.build, and some uses of matrix subreferencing.
    (C(I,J) when I and/or J are unsorted lists).

