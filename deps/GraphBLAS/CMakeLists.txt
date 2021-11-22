#-------------------------------------------------------------------------------
# GraphBLAS/CMakeLists.txt:  cmake script for GraphBLAS
#-------------------------------------------------------------------------------

# SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# See the User Guide for details on how to compile SuiteSparse:GraphBLAS.

#-------------------------------------------------------------------------------
# get the version
#-------------------------------------------------------------------------------

# cmake 3.13 is required; cmake 3.17 or later required for CUDA
cmake_minimum_required ( VERSION 3.13 )

message ( STATUS "CMake version: " ${CMAKE_VERSION} )

if ( CMAKE_VERSION VERSION_GREATER "3.0" )
    cmake_policy ( SET CMP0042 NEW )
    cmake_policy ( SET CMP0048 NEW )
    cmake_policy ( SET CMP0054 NEW )
endif ( )
set ( CMAKE_MACOSX_RPATH TRUE )

# version of SuiteSparse:GraphBLAS
set ( GraphBLAS_DATE "Nov 15, 2021")
set ( GraphBLAS_VERSION_MAJOR 6 )
set ( GraphBLAS_VERSION_MINOR 0 )
set ( GraphBLAS_VERSION_SUB   0 )

message ( STATUS "Building SuiteSparse:GraphBLAS version: v" ${GraphBLAS_VERSION_MAJOR}.${GraphBLAS_VERSION_MINOR}.${GraphBLAS_VERSION_SUB}  " date: " ${GraphBLAS_DATE} )

# GraphBLAS C API Specification version, at graphblas.org
if ( GraphBLAS_VERSION_MAJOR GREATER_EQUAL 6 )
    # SuiteSparse:GraphBLAS v6.0.0 and later fully implment the v2 spec
    set ( GraphBLAS_API_DATE "Nov 15, 2021" )
    set ( GraphBLAS_API_VERSION_MAJOR 2 )
    set ( GraphBLAS_API_VERSION_MINOR 0 )
    set ( GraphBLAS_API_VERSION_SUB   0 )
else ( )
    # SuiteSparse:GraphBLAS v5.2.0 has much of the v2 spec but not all.
    # It is backward compatible with all v5.x which have little of the new
    # features of the v2 C API Specification.
    set ( GraphBLAS_API_DATE "Sept 25, 2019" )
    set ( GraphBLAS_API_VERSION_MAJOR 1 )
    set ( GraphBLAS_API_VERSION_MINOR 3 )
    set ( GraphBLAS_API_VERSION_SUB   0 )
endif ( )

message ( STATUS "GraphBLAS C API: v" "${GraphBLAS_API_VERSION_MAJOR}.${GraphBLAS_API_VERSION_MINOR}, date: ${GraphBLAS_API_DATE}" )

project ( graphblas
        VERSION "${GraphBLAS_VERSION_MAJOR}.${GraphBLAS_VERSION_MINOR}.${GraphBLAS_VERSION_SUB}" LANGUAGES C )

#-------------------------------------------------------------------------------
# determine build type
#-------------------------------------------------------------------------------

include ( GNUInstallDirs )

# Uncomment this line for for development only, not for end-users:
# set ( CMAKE_BUILD_TYPE Debug )

if ( NOT CMAKE_BUILD_TYPE )
    set ( CMAKE_BUILD_TYPE Release )
endif ( )

# select "true" to build both dynamic and static libraries:
# set ( BUILD_GRB_STATIC_LIBRARY true )
# set ( BUILD_GRB_STATIC_LIBRARY false )
# or use cmake with -DBUILD_GRB_STATIC_LIBRARY=1

if ( BUILD_GRB_STATIC_LIBRARY )
    message ( STATUS "Building all GraphBLAS libraries (static and dynamic)" )
else ( )
    message ( STATUS "Building dynamic GraphBLAS library" )
endif ( )

# Uncomment this line for development only, not for end-users:
# set ( GBCOMPACT true )

if ( GBCOMPACT )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DGBCOMPACT=1 " )
endif ( )

#-------------------------------------------------------------------------------
# Configure Include/GraphBLAS.h and documentation with version number
#-------------------------------------------------------------------------------

configure_file (
    "Config/GraphBLAS.h.in"
    "${PROJECT_SOURCE_DIR}/Include/GraphBLAS.h"
)
configure_file (
    "Config/GraphBLAS_version.tex.in"
    "${PROJECT_SOURCE_DIR}/Doc/GraphBLAS_version.tex"
)
configure_file (
    "Config/GraphBLAS_API_version.tex.in"
    "${PROJECT_SOURCE_DIR}/Doc/GraphBLAS_API_version.tex"
)
configure_file (
    "Config/README.md.in"
    "${PROJECT_SOURCE_DIR}/README.md"
)

#-------------------------------------------------------------------------------
# find packages
#-------------------------------------------------------------------------------

# include ( FindOpenMP  )
find_package ( OpenMP )

# Intel(R) IPP not yet supported
# find_package ( IPP )

if ( ${CMAKE_VERSION} VERSION_LESS "3.17.0" )
    set ( CMAKE_CUDA off )
    set ( RMM_WRAP off )
    message ( STATUS "CUDA: not enabled (cmake 3.17.0 or later required)" )
else ( )
    include ( CheckLanguage )
    check_language ( CUDA )
    if ( CMAKE_CUDA_COMPILER )
        # with CUDA:
        include ( FindCUDAToolkit )
        message ( STATUS "CUDA toolkit found:   " ${CUDAToolkit_FOUND} )
        message ( STATUS "CUDA toolkit version: " ${CUDAToolkit_VERSION} )
        if ( CUDAToolkit_VERSION VERSION_LESS "11.2" )
            message ( STATUS "CUDA: not enabled (CUDA 11.2 or later required)" )
            set ( CMAKE_CUDA off )
            set ( RMM_WRAP off )
        else ( )
            enable_language ( CUDA )
            message ( STATUS "CUDA: enabled")
            set ( CMAKE_CUDA on )
            set ( RMM_WRAP on )
        endif ()
    else ( )
        # without CUDA:
        message ( STATUS "CUDA: not enabled")
        set ( CMAKE_CUDA off )
        set ( RMM_WRAP off )
    endif ( )
endif ( )

# TODO: CUDA disabled for now
set ( CMAKE_CUDA off )
set ( RMM_WRAP off )

if ( CMAKE_CUDA )
    # find_package (CUDA REQUIRED )
    set ( CMAKE_CUDA_FLAG " -DGBCUDA" )
    add_subdirectory(CUDA)
    set ( GB_CUDA graphblascuda  ${CUDA_LIBRARIES})
    link_directories ( "CUDA" ${CUDA_LIBRARIES})

    # TODO: Need to be able to enable this. For now, use make already in ${REPO_ROOT}/CUDA
    #add_subdirectory("CUDA")
else ( )
    set ( CMAKE_CUDA_FLAG " " )
    set ( GB_CUDA )
endif ( )

# TODO: Eventually this should be nested under CMAKE_CUDA conditional(?)
if ( RMM_WRAP )
    message ( STATUS "RMM enabled" )
    set ( CMAKE_RMM_FLAG " -DGBRMM" )
    set ( GB_RMM rmm_wrap ${CUDA_LIBRARIES} stdc++ )
    add_subdirectory(rmm_wrap)
    include_directories ( "rmm_wrap" ${CUDA_INCLUDE_DIRS})
    link_directories ( "CUDA" "${CUDA_LIBRARIES}" "/usr/local/cuda/lib64/stubs" "rmm_wrap"  )
else ( )
    message ( STATUS "RMM not enabled" )
    set ( CMAKE_RMM_FLAG " " )
    set ( GB_RMM )
endif ( )

#-------------------------------------------------------------------------------
# find the BLAS
#-------------------------------------------------------------------------------

#   if ( CMAKE_VERSION VERSION_GREATER "3.13" )
#       # Look for the parallel 64-bit MKL BLAS by default
#       set ( BLA_VENDOR OpenBLAS )
#       include ( FindBLAS )
#       if ( ${BLAS_FOUND} )
#           message ( STATUS "CBLAS : found" )
#           set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -lopenblas " )
#       else ( )
#           message ( STATUS "CBLAS : not found" )
#           # FUTURE: enable other BLAS flavors here
#           # set ( BLA_VENDOR ... )
#           # include ( FindBLAS )
#       endif ( )
#   endif ( )

#   if ( ${BLAS_FOUND} )
#       # BLAS_LINKER_FLAGS: uncached list of required linker flags
#       # (excluding -l and -L).
#       message ( STATUS "CBLAS:                     found" )
#       message ( STATUS "CBLAS linker flags:        " ${BLAS_LINKER_FLAGS} )
#       # BLAS_LIBRARIES: list of libraries to link against (may be empty)
#       message ( STATUS "CBLAS libraries:    " ${BLAS_LIBRARIES} )
#   else ( )
#       message ( STATUS "CBLAS:                     not found" )
#   endif ( )

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
message ( STATUS "CMAKE have OpenMP:         " ${OPENMP_FOUND} )

#-------------------------------------------------------------------------------
# include directories for both graphblas and graphblasdemo libraries
#-------------------------------------------------------------------------------

set ( CMAKE_INCLUDE_CURRENT_DIR ON )

if ( GBCOMPACT )
    include_directories ( Source/Template Source Include Source/Generated1 lz4 Demo/Include )
else ( )
    include_directories ( Source/Template Source Include Source/Generated1 lz4 Source/Generated2 Demo/Include )
endif ( )

#-------------------------------------------------------------------------------
# compiler options
#-------------------------------------------------------------------------------

# check which compiler is being used.  If you need to make
# compiler-specific modifications, here is the place to do it.
if ( "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    # The -g option is useful for the Intel(R) VTune tool, but it should be
    # removed in production.  Comment this line out if not in use:
    # set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g" )
    # cmake 2.8 workaround: gcc needs to be told to do ANSI C11.
    # cmake 3.0 doesn't have this problem.
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c11 -lm -Wno-pragmas " )
    # operations may be carried out in higher precision
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fexcess-precision=fast " )
    # faster single complex multiplication and division
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fcx-limited-range " )
    # math functions do not need to report errno
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fno-math-errno ")
    # integer operations wrap
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fwrapv ")
    # check all warnings (uncomment for development only)
#   set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror " )
    if ( CMAKE_C_COMPILER_VERSION VERSION_LESS 4.9 )
        message ( FATAL_ERROR "gcc version must be at least 4.9" )
    endif ( )
elseif ( "${CMAKE_C_COMPILER_ID}" STREQUAL "Intel" )
    # options for icc: also needs -std=c11
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -diag-disable 10397,15552 " )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -qopt-report=5 -qopt-report-phase=vec" )
    # the -mp1 option is important for predictable floating-point results with
    # the icc compiler.  Without, ((float) 1.)/((float) 0.) produces NaN,
    # instead of the correct result, Inf.
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c11 -mp1" )
    # The -g option is useful for the Intel VTune tool, but it should be
    # removed in production.  Comment this line out if not in use:
    # set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g" )
#   set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -qopt-malloc-options=3" )
    # check all warnings and remarks (uncomment for development only):
#   set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -w3 -Wremarks -Werror " )
    if ( CMAKE_C_COMPILER_VERSION VERSION_LESS 19.0 )
        message ( FATAL_ERROR "icc version must be at least 19.0" )
    endif ( )
elseif ( "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" )
    # options for clang
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wno-pointer-sign " )
    if ( CMAKE_C_COMPILER_VERSION VERSION_LESS 3.3 )
        message ( FATAL_ERROR "clang version must be at least 3.3" )
    endif ( )
elseif ( "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC" )
    # options for MicroSoft Visual Studio
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2 -wd\"4244\" -wd\"4146\" -wd\"4018\" -wd\"4996\" -wd\"4047\" -wd\"4554\"")
elseif ( "${CMAKE_C_COMPILER_ID}" STREQUAL "PGI" )
    # options for PGI pgcc compiler.  The compiler has a bug, and the
    # -DPGI_COMPILER_BUG causes GraphBLAS to use a workaround.
    set ( CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -Mnoopenmp -noswitcherror -c11 -lm -DPGI_COMPILER_BUG" )
    set ( CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS} -Mnoopenmp -D__GCC_ATOMIC_TEST_AND_SET_TRUEVAL=1 -noswitcherror --c++11 -lm -DPGI_COMPILER_BUG" )
endif ( )

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}" )
else ( )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
endif ( )

set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_CUDA_FLAG}" )

#-------------------------------------------------------------------------------
# dynamic graphblas library properties
#-------------------------------------------------------------------------------

# Notes from Sebastien Villemot (sebastien@debian.org):
# SOVERSION policy: if a binary compiled against the old version of the shared
# library needs recompiling in order to work with the new version, then a
# SO_VERSION increase # is needed. Otherwise not.  Examples of the changes that
# require a SO_VERSION increase:
#
#   - a public function or static variable is removed
#   - the prototype of a public function changes
#   - the integer value attached to a public #define or enum changes
#   - the fields of a public structure are modified
#
# Examples of changes that do not require a SO_VERSION increase:
#
#   - a new public function or static variable is added
#   - a private function or static variable is removed or modified
#   - changes in the internals of a structure that is opaque to the calling
#       program (i.e. is only a pointer manipulated through public functions of
#       the library)
#   - a public enum is extended (by adding a new item at the end, but without
#       changing the already existing items)

if ( GBCOMPACT )
    file ( GLOB GRAPHBLAS_SOURCES "Source/*.c" "Source/Generated1/*.c" )
else ( )
    file ( GLOB GRAPHBLAS_SOURCES "Source/*.c" "Source/Generated1/*.c" "Source/Generated2/*.c" )
endif ( )

add_library ( graphblas SHARED ${GRAPHBLAS_SOURCES} )

if ( CMAKE_CUDA )
    add_dependencies(graphblas graphblascuda)
endif()

if ( RMM_WRAP )
    add_dependencies(graphblas rmm_wrap)
endif()

SET_TARGET_PROPERTIES ( graphblas PROPERTIES
    VERSION ${GraphBLAS_VERSION_MAJOR}.${GraphBLAS_VERSION_MINOR}.${GraphBLAS_VERSION_SUB}
    SOVERSION ${GraphBLAS_VERSION_MAJOR}
    C_STANDARD_REQUIRED 11
    PUBLIC_HEADER "Include/GraphBLAS.h" )
set_property ( TARGET graphblas PROPERTY C_STANDARD 11 )

#-------------------------------------------------------------------------------
# static graphblas library properties
#-------------------------------------------------------------------------------

if ( BUILD_GRB_STATIC_LIBRARY )
    add_library ( graphblas_static STATIC ${GRAPHBLAS_SOURCES} )
    SET_TARGET_PROPERTIES ( graphblas_static PROPERTIES
       VERSION ${GraphBLAS_VERSION_MAJOR}.${GraphBLAS_VERSION_MINOR}.${GraphBLAS_VERSION_SUB}
       OUTPUT_NAME graphblas
       SOVERSION ${GraphBLAS_VERSION_MAJOR}
       C_STANDARD_REQUIRED 11
       PUBLIC_HEADER "Include/GraphBLAS.h" )
   set_property ( TARGET graphblas_static PROPERTY C_STANDARD 11 )
endif ( )

#-------------------------------------------------------------------------------
# select the threading library
#-------------------------------------------------------------------------------

if ( OPENMP_FOUND )
#   set this to 'false' if you do not want OpenMP
#   set ( USE_OPENMP false )
    set ( USE_OPENMP true )
endif ( )

#-------------------------------------------------------------------------------
# select the math library (not required for Microsoft Visual Studio)
#-------------------------------------------------------------------------------

if ( "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC" )
    set ( M_LIB "" )
else ( )
    set ( M_LIB "m" )
endif ( )

target_link_libraries ( graphblas PUBLIC ${M_LIB} )
if ( BUILD_GRB_STATIC_LIBRARY )
    target_link_libraries ( graphblas_static PUBLIC ${M_LIB} )
endif ( )

#-------------------------------------------------------------------------------
# add the OpenMP, IPP, CUDA, BLAS, etc libraries
#-------------------------------------------------------------------------------

if ( USE_OPENMP )
    message ( STATUS "CMAKE OpenMP libraries:    " ${OpenMP_C_LIBRARIES} )
    message ( STATUS "CMAKE OpenMP include:      " ${OpenMP_C_INCLUDE_DIRS} )
    target_link_libraries ( graphblas PUBLIC ${OpenMP_C_LIBRARIES} )
    if ( BUILD_GRB_STATIC_LIBRARY )
        target_link_libraries ( graphblas_static PUBLIC ${OpenMP_C_LIBRARIES} )
    endif ( )
    message ( STATUS "CMAKE OpenMP C flags:      " ${OpenMP_C_FLAGS} )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} " )
    include_directories ( ${OpenMP_C_INCLUDE_DIRS} )
endif ( )

# Intel(R) IPP not yet supported
#   message ( STATUS "CMAKE have IPP:            " ${IPP_FOUND} )
#   if ( IPP_FOUND )
#       message ( STATUS "Intel(R) IPP variant:      " ${IPP_TL_VARIANT} )
#       message ( STATUS "Intel(R) IPP arch:         " ${IPP_ARCH} )
#       message ( STATUS "Intel(R) IPP version:      " ${IPP_VERSION} )
#       set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DGB_HAVE_IPP=1 " )
#       target_link_libraries ( graphblas PUBLIC ippdc ippcore )
#       if ( BUILD_GRB_STATIC_LIBRARY )
#           target_link_libraries ( graphblas_static PUBLIC ippdc ippcore )
#       endif ( )
#   endif ( )

if ( CMAKE_CUDA )
    target_link_libraries ( graphblas PUBLIC ${GB_CUDA} ${GB_RMM} )
    if ( BUILD_GRB_STATIC_LIBRARY )
        target_link_libraries ( graphblas_static PUBLIC ${GB_CUDA} ${GB_RMM} )
    endif ( )
endif ( )

#-------------------------------------------------------------------------------
# add the BLAS
#-------------------------------------------------------------------------------

# if ( BLAS_FOUND )
#     # use the dense CBLAS
#     message ( STATUS "Using dense CBLAS for faster dense matrix/vector operations" )
#     set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DGB_HAS_CBLAS " )
#     target_link_libraries ( graphblas PUBLIC ${BLAS_LIBRARIES} )
# endif ( )

#-------------------------------------------------------------------------------
# print final C flags
#-------------------------------------------------------------------------------

message ( STATUS "CMAKE C flags: " ${CMAKE_C_FLAGS} )

#-------------------------------------------------------------------------------
# Demo library and programs
#-------------------------------------------------------------------------------

if ( DEMO )

    #---------------------------------------------------------------------------
    # demo library
    #---------------------------------------------------------------------------

    message ( STATUS "Also compiling the demos in GraphBLAS/Demo" )

    file ( GLOB DEMO_SOURCES "Demo/Source/*.c" )
    add_library ( graphblasdemo SHARED ${DEMO_SOURCES} )

    SET_TARGET_PROPERTIES ( graphblasdemo PROPERTIES
        VERSION ${GraphBLAS_VERSION_MAJOR}.${GraphBLAS_VERSION_MINOR}.${GraphBLAS_VERSION_SUB}
        SOVERSION ${GraphBLAS_VERSION_MAJOR}
        C_STANDARD_REQUIRED 11 )
    set_property ( TARGET graphblasdemo PROPERTY C_STANDARD 11 )
    target_link_libraries ( graphblasdemo PUBLIC ${M_LIB} graphblas  ${GB_CUDA} ${GB_RMM})

    if ( BUILD_GRB_STATIC_LIBRARY )
        add_library ( graphblasdemo_static STATIC ${DEMO_SOURCES} )
        SET_TARGET_PROPERTIES ( graphblasdemo_static PROPERTIES
            VERSION ${GraphBLAS_VERSION_MAJOR}.${GraphBLAS_VERSION_MINOR}.${GraphBLAS_VERSION_SUB}
            C_STANDARD_REQUIRED 11 )
        set_property ( TARGET graphblasdemo_static PROPERTY C_STANDARD 11 )
        target_link_libraries ( graphblasdemo_static PUBLIC  graphblas_static  ${GB_CUDA} ${GB_RMM} )
    endif ( )

    #---------------------------------------------------------------------------
    # Demo programs
    #---------------------------------------------------------------------------

    add_executable ( openmp_demo   "Demo/Program/openmp_demo.c" )
    add_executable ( complex_demo  "Demo/Program/complex_demo.c" )
    add_executable ( kron_demo     "Demo/Program/kron_demo.c" )
    add_executable ( simple_demo   "Demo/Program/simple_demo.c" )
    add_executable ( wildtype_demo "Demo/Program/wildtype_demo.c" )
    add_executable ( reduce_demo   "Demo/Program/reduce_demo.c" )
    add_executable ( import_demo   "Demo/Program/import_demo.c" )

    # Libraries required for Demo programs
    target_link_libraries ( openmp_demo   PUBLIC graphblas graphblasdemo ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( complex_demo  PUBLIC graphblas graphblasdemo ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( kron_demo     PUBLIC graphblas graphblasdemo ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( simple_demo   PUBLIC graphblasdemo ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( wildtype_demo PUBLIC graphblas ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( reduce_demo   PUBLIC graphblas ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( import_demo   PUBLIC graphblas graphblasdemo ${GB_CUDA} ${GB_RMM} )

else ( )

    message ( STATUS "Skipping the demos in GraphBLAS/Demo" )

endif ( )

#-------------------------------------------------------------------------------
# graphblas installation location
#-------------------------------------------------------------------------------

if ( BUILD_GRB_STATIC_LIBRARY )

    # install both the dynamic and static libraries
    install ( TARGETS graphblas  graphblas_static
        LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR} )

else ( )

    # only install the dynamic graphblas library
    install ( TARGETS graphblas
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR} )

endif ( )

#-------------------------------------------------------------------------------
# dump all variables, for debugging only
#-------------------------------------------------------------------------------

# uncomment this line to dump all cmake variables:
# set ( DUMP true )

if ( DUMP )
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endif ( )
