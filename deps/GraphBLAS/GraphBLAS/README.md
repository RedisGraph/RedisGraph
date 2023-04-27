# GraphBLAS/GraphBLAS: Octave/MATLAB interface for SuiteSparse:GraphBLAS

SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
The @GrB interface (including its test suite and demos) is under the GNU
GPLv3 (or later) license.
SPDX-License-Identifier: GPL-3.0-or-later
You may not use the @GrB interface under the Apache-2.0 license.

The @GrB class provides an easy-to-use interface to SuiteSparse:GraphBLAS.

To install it for use in Octave/MATLAB, first compile the GraphBLAS library,
-lgraphblas (or -lgraphblas_renamed for MATLAB R2021a and later).  See the
instructions in the top-level GraphBLAS folder for details.  Be sure to use
OpenMP for best performance.  The default installation process places the
GraphBLAS library in /usr/local/lib.  If you do not have root access and cannot
install GraphBLAS into /usr/local/lib, then follow the instructions below to
modify your library path, but instead of /usr/local/lib, use
/home/me/SuiteSparse/GraphBLAS/build, where "/home/me/SuiteSparse/GraphBLAS" is
where you placed your copy of GraphBLAS.

If you have MATLAB R2021a, the gbmake script will link against the library
-lgraphblas_renamed, not -lgraphblas, because that version of MATLAB includes
its own version of SuiteSparse:GraphBLAS (v3.3.3, an earlier one).  To avoid a
name conflict, you must compile the -lgraphblas_renamed library in
/home/me/SuiteSparse/GraphBLAS/GraphBLAS/build.

Octave/MATLAB needs to know where to find the compiled GraphBLAS library.  On
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

If you don't have system priveledges to change /usr/local/lib, then add the
build folder to your LD_LIBRARY_PATH instead, either.  For Octave, and MATLAB
R2020b and earlier: /home/me/SuiteSparse/GraphBLAS/build for libgraphblas.so,
For R2021a:  /home/me/SuiteSparse/GraphBLAS/GraphBLAS/build for
libgraphblas_renamed.so.

On Windows 10, on the Search bar type env and hit enter; (or you can
right-click My Computer or This PC and select Properties, and then select
Advanced System Settings).  Select "Edit the system environment variables",
then "Environment Variables".  Under "System Variables" select "Path" and click
"Edit".  These "New" to add a path and then "Browse".  Browse to the folder
(for example: C:/Users/me/Documents/SuiteSparse/GraphBLAS/build/Release) and
add it to your path.  For MATLAB R2021a and later, you must use the
libgraphblas_renamed.dll, in:
/User/me/SuiteSparse/GraphBLAS/GraphBLAS/build/Release instead.  Then close the
editor, sign out of Windows and sign back in again.

Next, start Octave/MATLAB and go to this GraphBLAS/GraphBLAS folder.  Type

    addpath (pwd)

to add the GraphBLAS interface to your path.  Then do

    savepath

Or, if that function is not allowed because of file permissions, add this
command to your startup.m file:

    % add the Octave/MATLAB interface to the Octave/MATLAB path
    addpath ('/home/me/SuiteSparse/GraphBLAS/GraphBLAS') :

where the path /home/me/SuiteSparse/GraphBLAS/GraphBLAS is the full path to
this folder.

The name "GraphBLAS/GraphBLAS" is used for this folder so that this can be done
in Octave/MATLAB:

    help GraphBLAS

To get additional help, type:

    methods GrB
    help GrB

Next, go to the GraphBLAS/GraphBLAS/@GrB/private folder and compile the
Octave/MATLAB mexFunctions.  Assuming your working directory is
GraphBLAS/GraphBLAS (where this README.md file is located), do the following:

    cd @GrB/private
    gbmake

To run the demos, go to the GraphBLAS/GraphBLAS/demo folder and type:

    gbdemo
    gbdemo2

The output of these demos on a Dell XPS 13 laptop and an NVIDIA DGX Station can
also be found in GraphBLAS/GraphBLAS/demo/html, in both PDF and HTML formats.

To test your installation, go to GraphBLAS/GraphBLAS/test and type:

    gbtest

If everything is successful, it should report 'gbtest: all tests passed'.  Note
that gbtest tests all features of the Octave/MATLAB interface to
SuiteSparse/GraphBLAS, including error handling, so you can expect to see error
messages during the test.  This is expected.

# FUTURE: Not yet supported for GrB matrices in Octave/MATLAB:

    linear indexing
    2nd output for [x,i] = max (...) and [x,i] = min (...); needs
        modified reduction methods inside GraphBLAS
    'includenan' for min and max
    min and max for complex matrices
    singleton expansion
    3D and higher dimensional matrices:
        this might be done by converting the higher dimensioal
        indices down to a large 2D space, and relying on hypersparsity.
    saturating element-wise binary and unary operators for integers.
        See also the discussion in the User Guide.

These functions are supported, but are not yet as fast as they could be:
bandwidth, eps, isbanded, isdiag, ishermitian, issymmetric, istril, istriu,
spfun.

