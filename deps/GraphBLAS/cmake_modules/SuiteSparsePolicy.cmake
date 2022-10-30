#-------------------------------------------------------------------------------
# SuiteSparse/SuiteSparse_config/cmake_modules/SuiteSparsePolicy.cmake
#-------------------------------------------------------------------------------

# Copyright (c) 2022, Timothy A. Davis.  All Rights Reserved.
# SPDX-License-Identifier: BSD-3-clause

#-------------------------------------------------------------------------------

# SuiteSparse CMake policies.  The following parameters can be defined prior
# to including this file:
#
#   CMAKE_BUILD_TYPE:   if not set, it is set below to "Release".
#                       To use the "Debug" policy, precede this with
#                       set ( CMAKE_BUILD_TYPE Debug )
#
#   ENABLE_CUDA:        if set to true, CUDA is enabled for the project.
#
# To select a specific BLAS library, edit this file and uncomment one of:
# set ( BLA_VENDOR ... )

cmake_minimum_required ( VERSION 3.19 )

message ( STATUS "Source:        ${CMAKE_SOURCE_DIR} ")
message ( STATUS "Build:         ${CMAKE_BINARY_DIR} ")

cmake_policy ( SET CMP0042 NEW )    # enable MACOSX_RPATH by default
cmake_policy ( SET CMP0048 NEW )    # VERSION variable policy
cmake_policy ( SET CMP0054 NEW )    # if ( expression ) handling policy

set ( CMAKE_MACOSX_RPATH TRUE )
enable_language ( C )
include ( GNUInstallDirs )

# add the ./build folder to the runpath so other SuiteSparse packages can
# find this one without "make install"
set ( CMAKE_BUILD_RPATH ${CMAKE_BUILD_RPATH} ${CMAKE_BINARY_DIR} )

# determine if this package is inside the top-level SuiteSparse folder
# (if ../lib and ../include exist, relative to the source directory)

if ( SUITESPARSE_SECOND_LEVEL )
    # the package is normally located at the 2nd level inside SuiteSparse
    # (SuiteSparse/GraphBLAS/GraphBLAS/ for example)
    set ( INSIDE_SUITESPARSE 
            ( ( EXISTS ${CMAKE_SOURCE_DIR}/../../lib     ) AND 
            (   EXISTS ${CMAKE_SOURCE_DIR}/../../include ) ) )
else ( )
    # typical case, the package is at the 1st level inside SuiteSparse
    # (SuiteSparse/AMD for example)
    set ( INSIDE_SUITESPARSE 
            ( ( EXISTS ${CMAKE_SOURCE_DIR}/../lib     ) AND 
            (   EXISTS ${CMAKE_SOURCE_DIR}/../include ) ) )
endif ( )

if ( INSIDE_SUITESPARSE )
    # ../lib and ../include exist: the package is inside SuiteSparse.
    # find ( REAL_PATH ...) requires cmake 3.19.
    if ( SUITESPARSE_SECOND_LEVEL )
        file ( REAL_PATH  ${CMAKE_SOURCE_DIR}/../../lib     SUITESPARSE_LIBDIR )
        file ( REAL_PATH  ${CMAKE_SOURCE_DIR}/../../include SUITESPARSE_INCLUDEDIR )
        file ( REAL_PATH  ${CMAKE_SOURCE_DIR}/../../bin     SUITESPARSE_BINDIR )
    else ( )
        file ( REAL_PATH  ${CMAKE_SOURCE_DIR}/../lib     SUITESPARSE_LIBDIR )
        file ( REAL_PATH  ${CMAKE_SOURCE_DIR}/../include SUITESPARSE_INCLUDEDIR )
        file ( REAL_PATH  ${CMAKE_SOURCE_DIR}/../bin     SUITESPARSE_BINDIR )
    endif ( )
    message ( STATUS "Local install: ${SUITESPARSE_LIBDIR} ")
    message ( STATUS "Local include: ${SUITESPARSE_INCLUDEDIR} ")
    message ( STATUS "Local bin:     ${SUITESPARSE_BINDIR} ")
    # append ../lib to the install and build runpaths 
    set ( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${SUITESPARSE_LIBDIR} )
    set ( CMAKE_BUILD_RPATH   ${CMAKE_BUILD_RPATH}   ${SUITESPARSE_LIBDIR} )
endif ( )

message ( STATUS "Install rpath: ${CMAKE_INSTALL_RPATH} ")
message ( STATUS "Build   rpath: ${CMAKE_BUILD_RPATH} ")

if ( NOT CMAKE_BUILD_TYPE )
    set ( CMAKE_BUILD_TYPE Release )
endif ( )

message ( STATUS "Build type:    ${CMAKE_BUILD_TYPE} ")

set ( CMAKE_INCLUDE_CURRENT_DIR ON )

#-------------------------------------------------------------------------------
# BLAS library
#-------------------------------------------------------------------------------

# Uncomment one of the lines below to select OpenBLAS or the Intel MKL.
# Be sure to use a parallel BLAS library for best results in UMFPACK,
# CHOLMOD, SPQR, and ParU.  All SuiteSparse packages should use the same
# BLAS and the same OpenMP library.

# set ( BLA_VENDOR OpenBLAS )       # OpenBLAS with 32-bit integers
# set ( BLA_VENDOR Intel10_64ilp )  # MKL BLAS with 64-bit integers
# set ( BLA_VENDOR Intel10_64lp )   # MKL BLAS with 32-bit integers

# The Intel MKL BLAS is recommended.  It is free to download (but be sure to
# check their license to make sure you accept it).   See:
# https://www.intel.com/content/www/us/en/developer/tools/oneapi/onemkl.htm

#-------------------------------------------------------------------------------
# find CUDA
#-------------------------------------------------------------------------------

if ( ENABLE_CUDA )

    # try finding CUDA
    include ( CheckLanguage )
    check_language ( CUDA )
    message ( STATUS "Looking for CUDA" )
    if ( CMAKE_CUDA_COMPILER )
        # with CUDA:
        message ( STATUS "Find CUDA tool kit:" )
        # FindCUDAToolKit needs to have C or CXX enabled first (see above)
        include ( FindCUDAToolkit )
        message ( STATUS "CUDA toolkit found:   " ${CUDAToolkit_FOUND} )
        message ( STATUS "CUDA toolkit version: " ${CUDAToolkit_VERSION} )
        message ( STATUS "CUDA toolkit include: " ${CUDAToolkit_INCLUDE_DIRS} )
        message ( STATUS "CUDA toolkit lib dir: " ${CUDAToolkit_LIBRARY_DIR} )
        if ( CUDAToolkit_VERSION VERSION_LESS "11.2" )
            # CUDA is present but too old
            message ( STATUS "CUDA: not enabled (CUDA 11.2 or later required)" )
            set ( SUITESPARSE_CUDA off )
        else ( )
            # CUDA 11.2 or later present
            enable_language ( CUDA )
            set ( SUITESPARSE_CUDA on )
        endif ( )
    else ( )
        # without CUDA:
        message ( STATUS "CUDA: not found" )
        set ( SUITESPARSE_CUDA off )
    endif ( )

else ( )

    # CUDA is disabled
    set ( SUITESPARSE_CUDA off )

endif ( )

if ( SUITESPARSE_CUDA )
    message ( STATUS "CUDA: enabled" )
else ( )
    message ( STATUS "CUDA: not enabled" )
endif ( )

