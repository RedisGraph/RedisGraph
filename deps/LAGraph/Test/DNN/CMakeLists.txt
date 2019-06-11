#-------------------------------------------------------------------------------
# LAGraph/Test/DNN/CMakeLists.txt:  cmake script for LAGraph test
#-------------------------------------------------------------------------------

# LAGraph, (... list all authors here) (c) 2019, All Rights Reserved.
# http://lagraph.org  See LAGraph/LICENSE for license.

# CMakeLists.txt: instructions for cmake to build LAGraph.
# An ANSI C11 compiler is required.
#
# First, install any GraphBLAS library.
# Alternatively, use ../GraphBLAS (see comments below).
#
# To compile and install the LAGraph library:
#
#   make
#   sudo make install
#
# Then to compile and run the Demos:
# 
#   cd Demos
#   make
#
# If that fails for any reason, make sure your compiler supports ANSI C11.  You
# could try changing your compiler, for example:
#
#   cd build
#   CC=icc cmake ..
#   cd ..
#   make
#
# Or, with other compilers:
#
#   CC=xlc cmake ..
#   CC=gcc cmake ..
#
# To remove all compiled files and libraries (except installed ones):
#
#   make distclean

#-------------------------------------------------------------------------------
# get the version
#-------------------------------------------------------------------------------

# cmake 3.0 is preferred.
cmake_minimum_required ( VERSION 2.8.12 )

if ( CMAKE_VERSION VERSION_GREATER "3.0" )
    cmake_policy ( SET CMP0042 NEW )
    cmake_policy ( SET CMP0048 NEW )
endif ( )

project ( dnn )

#-------------------------------------------------------------------------------
# determine build type
#-------------------------------------------------------------------------------

include ( GNUInstallDirs )

# For development only, not for end-users:
# set ( CMAKE_BUILD_TYPE Debug )

if ( NOT CMAKE_BUILD_TYPE )
    set ( CMAKE_BUILD_TYPE Release )
endif ( )

#-------------------------------------------------------------------------------
# edit these lines to select your GraphBLAS library
#-------------------------------------------------------------------------------

# link_directories ( /usr/local/lib )
link_directories ( ../../../GraphBLAS/build )
link_directories ( ../../build )

# include_directories ( /usr/local/include )
include_directories ( ../../../GraphBLAS/Include )
include_directories ( ../../Include )

#-------------------------------------------------------------------------------
# determine what user threading model to use
#-------------------------------------------------------------------------------

include ( FindOpenMP  )
include ( FindThreads )

#-------------------------------------------------------------------------------
# report status
#-------------------------------------------------------------------------------

message ( STATUS "CMAKE build type:          " ${CMAKE_BUILD_TYPE} )

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message ( STATUS "CMAKE C Flags debug:       " ${CMAKE_C_FLAGS_DEBUG} )
else ( )
    message ( STATUS "CMAKE C Flags release:     " ${CMAKE_C_FLAGS_RELEASE} )
endif ( )

message ( STATUS "CMAKE compiler ID:         " ${CMAKE_C_COMPILER_ID} )
message ( STATUS "CMAKE thread library:      " ${CMAKE_THREAD_LIBS_INIT} )
message ( STATUS "CMAKE have pthreads:       " ${CMAKE_USE_PTHREADS_INIT}  )
message ( STATUS "CMAKE have Win32 pthreads: " ${CMAKE_USE_WIN32_THREADS_INIT} )
message ( STATUS "CMAKE have OpenMP:         " ${OPENMP_FOUND} )

#-------------------------------------------------------------------------------
# include directories
#-------------------------------------------------------------------------------

set ( CMAKE_INCLUDE_CURRENT_DIR ON )

#-------------------------------------------------------------------------------
# compiler options
#-------------------------------------------------------------------------------

# check which compiler is being used.  If you need to make
# compiler-specific modifications, here is the place to do it.
if ( "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    # cmake 2.8 workaround: gcc needs to be told to do ANSI C11.
    # cmake 3.0 doesn't have this problem.
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c11 -lm -Wno-pragmas " )
    # check all warnings:
#   set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror " )
    if ( CMAKE_C_COMPILER_VERSION VERSION_LESS 4.9 )
        message ( FATAL_ERROR "gcc version must be at least 4.9" )
    endif ( )
elseif ( "${CMAKE_C_COMPILER_ID}" STREQUAL "Intel" )
    # options for icc: also needs -std=c11
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c11 " )
    # check all warnings:
#   set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -w3 " )
    if ( CMAKE_C_COMPILER_VERSION VERSION_LESS 18.0 )
        message ( FATAL_ERROR "icc version must be at least 18.0" )
    endif ( )
elseif ( "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" )
    # options for clang
    if ( CMAKE_C_COMPILER_VERSION VERSION_LESS 3.3 )
        message ( FATAL_ERROR "clang version must be at least 3.3" )
    endif ( )
elseif ( "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC" )
    # options for MicroSoft Visual Studio
endif ( )

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}" )
else ( )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}" )
endif ( )

#-------------------------------------------------------------------------------
# select the threading library 
#-------------------------------------------------------------------------------

if ( USER_OPENMP )
    # user insists on OpenMP synchronization inside LAGraph
    message ( STATUS "cmake -DUSER_OPENMP=1: insisting on using OpenMP" )
    set ( USE_OPENMP true )
elseif ( USER_POSIX )
    # user insists on POSIX synchronization inside LAGraph
    message ( STATUS "cmake -DUSER_POSIX=1: insisting on using POSIX" )
    set ( USE_POSIX true )
elseif ( USER_NONE )
    message ( STATUS "cmake -DUSER_NONE=1: insisting on using no threading" )
    set ( USE_NONE true )
else ( )
    # default: automatic selection
    message ( STATUS "Automatic selection of synchronization method" )
    if ( OPENMP_FOUND )
        set ( USE_OPENMP true )
    elseif ( CMAKE_USE_PTHREADS_INIT )
        set ( USE_POSIX true )
    endif ( )
endif ( )

# add the threading library
if ( USE_OPENMP )
    # use OpenMP
    message ( STATUS "Using OpenMP to synchronize user threads" )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -DUSER_OPENMP_THREADS " )
elseif ( USE_POSIX )
    # use POSIX
    message ( STATUS "Using POSIX pthreads to synchronize user threads" )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -pthread -DUSER_POSIX_THREADS " )
else ( )
    # use no threading at all
    message ( WARNING "No support for user threads; LAGraph will not be thread-safe" )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DUSER_NO_THREADS " )
endif ( )

if ( CMAKE_USE_PTHREADS_INIT )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DHAVE_PTHREADS " )
endif ( )

if ( CMAKE_USE_WIN32_THREADS_INIT )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DHAVE_WINDOWS_THREADS " )
endif ( )

#-------------------------------------------------------------------------------
# print final C flags
#-------------------------------------------------------------------------------

message ( STATUS "CMAKE C flags: " ${CMAKE_C_FLAGS} )

#-------------------------------------------------------------------------------
# Demo programs
#-------------------------------------------------------------------------------

file ( GLOB SOURCES "*.c" )

add_executable ( dnn ${SOURCES} )

# Libraries required for Demo programs
target_link_libraries ( dnn graphblas lagraph m )

