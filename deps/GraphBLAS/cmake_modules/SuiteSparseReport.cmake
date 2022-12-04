#-------------------------------------------------------------------------------
# SuiteSparse/SuiteSparse_config/SuiteSparseReport.cmake
#-------------------------------------------------------------------------------

# Copyright (c) 2022, Timothy A. Davis.  All Rights Reserved.
# SPDX-License-Identifier: BSD-3-clause

#-------------------------------------------------------------------------------
# report status and compile flags
#-------------------------------------------------------------------------------

message ( STATUS "CMAKE C flags:     ${CMAKE_C_FLAGS}" )
message ( STATUS "CMAKE C++ flags:   ${CMAKE_CXX_FLAGS}" )
message ( STATUS "CMAKE build type:  ${CMAKE_BUILD_TYPE}" )
if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
    message ( STATUS "CMAKE C Flags debug:     ${CMAKE_C_FLAGS_DEBUG} ")
    message ( STATUS "CMAKE C++ Flags debug:   ${CMAKE_CXX_FLAGS_DEBUG} ")
else ( )
    message ( STATUS "CMAKE C Flags release:   ${CMAKE_C_FLAGS_RELEASE} ")
    message ( STATUS "CMAKE C++ Flags release: ${CMAKE_CXX_FLAGS_RELEASE} ")
endif ( )
message ( STATUS "CMAKE C compiler:        ${CMAKE_C_COMPILER_ID} ")
message ( STATUS "CMAKE C++ compiler:      ${CMAKE_CXX_COMPILER_ID}" )
message ( STATUS "CMAKE have OpenMP:       ${OPENMP_FOUND} ")

