#-------------------------------------------------------------------------------
# GraphBLAS/CMakeLists.txt:  cmake script for GraphBLAS
#-------------------------------------------------------------------------------

# SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# See the User Guide for details on how to compile SuiteSparse:GraphBLAS.

#-------------------------------------------------------------------------------
# get the version
#-------------------------------------------------------------------------------

cmake_minimum_required ( VERSION 3.19 )

# version of SuiteSparse:GraphBLAS
set ( GraphBLAS_DATE "Oct 14, 2022" )
set ( GraphBLAS_VERSION_MAJOR 7 )
set ( GraphBLAS_VERSION_MINOR 3 )
set ( GraphBLAS_VERSION_SUB   0 )

# GraphBLAS C API Specification version, at graphblas.org
set ( GraphBLAS_API_DATE "Nov 15, 2021" )
set ( GraphBLAS_API_VERSION_MAJOR 2 )
set ( GraphBLAS_API_VERSION_MINOR 0 )
set ( GraphBLAS_API_VERSION_SUB   0 )

message ( STATUS "Building SuiteSparse:GraphBLAS version: v"
    ${GraphBLAS_VERSION_MAJOR}.${GraphBLAS_VERSION_MINOR}.${GraphBLAS_VERSION_SUB}
    ", date: " ${GraphBLAS_DATE} )

message ( STATUS "GraphBLAS C API: v"
    ${GraphBLAS_API_VERSION_MAJOR}.${GraphBLAS_API_VERSION_MINOR}
    ", date: ${GraphBLAS_API_DATE}" )

#-------------------------------------------------------------------------------
# SuiteSparse policies
#-------------------------------------------------------------------------------

set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/cmake_modules
    ${CMAKE_SOURCE_DIR}/../SuiteSparse_config/cmake_modules
    ${CMAKE_SOURCE_DIR}/../SuiteSparse/SuiteSparse_config/cmake_modules
    )

# CUDA is under development for now, and not deployed in production:
  set ( ENABLE_CUDA false )
# set ( ENABLE_CUDA true )
include ( SuiteSparsePolicy )

#-------------------------------------------------------------------------------
# define the project
#-------------------------------------------------------------------------------

if ( SUITESPARSE_CUDA )
    # for CUDA development only; not for production use
    set ( CMAKE_CUDA_DEV off )
    project ( graphblas
            VERSION "${GraphBLAS_VERSION_MAJOR}.${GraphBLAS_VERSION_MINOR}.${GraphBLAS_VERSION_SUB}"
            LANGUAGES CUDA C )
else ( )
    set ( CMAKE_CUDA_DEV off )
    project ( graphblas
            VERSION "${GraphBLAS_VERSION_MAJOR}.${GraphBLAS_VERSION_MINOR}.${GraphBLAS_VERSION_SUB}"
            LANGUAGES C )
endif ( )

#-------------------------------------------------------------------------------
# find OpenMP, CUDA, RMM, and cpu_features
#-------------------------------------------------------------------------------

find_package ( OpenMP )

if ( SUITESPARSE_CUDA )
    # with CUDA and RMM
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DGBCUDA" )
    add_subdirectory ( CUDA )
    set ( GB_CUDA graphblascuda  ${CUDA_LIBRARIES} )
    set ( GB_RMM rmm_wrap ${CUDA_LIBRARIES} )
    add_subdirectory ( rmm_wrap )
    include_directories ( "rmm_wrap" ${CUDA_INCLUDE_DIRS} )
    link_directories ( "CUDA" "${CUDA_LIBRARIES}" "/usr/local/cuda/lib64/stubs" "rmm_wrap" "/usr/local/cuda/lib64" )
else ( )
    # without CUDA and RMM
    set ( CMAKE_RMM_FLAG " " )
    set ( GB_CUDA )
    set ( GB_RMM )
endif ( )

if ( NOT GBNCPUFEAT )
    # default: enable Google's cpu_features package
    message ( STATUS "cpu_features (by google.com): enabled " )
    include_directories ( "cpu_features/include" "cpu_features" "cpu_features/src" "cpu_features/include/internal" )
else ( )
    # disable Google's cpu_features package
    message ( STATUS "cpu_features (by google.com): disabled" )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DGBNCPUFEAT " )
endif ( )

if ( DEFINED GBX86 )
    # default: this is detected automatically, but can be set here also
    if ( GBX86 )
        set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DGBX86=1 " )
    else ( )
        set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DGBX86=0 " )
    endif ( )
endif ( )

if ( DEFINED GBAVX2 )
    # default: this is detected automatically, but can be set here also
    if ( GBAVX2 )
        set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DGBAVX2=1 " )
    else ( )
        set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DGBAVX2=0 " )
    endif ( )
endif ( )

if ( DEFINED GBAVX512F )
    # default: this is detected automatically, but can be set here also
    if ( GBAVX512F )
        set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DGBAVX512F=1 " )
    else ( )
        set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DGBAVX512F=0 " )
    endif ( )
endif ( )

#-------------------------------------------------------------------------------
# determine build type
#-------------------------------------------------------------------------------

# select "true" to build both dynamic and static libraries:
# set ( BUILD_GRB_STATIC_LIBRARY true )
# set ( BUILD_GRB_STATIC_LIBRARY false )
# or use cmake with -DBUILD_GRB_STATIC_LIBRARY=1

if ( BUILD_GRB_STATIC_LIBRARY )
    message ( STATUS "Building all GraphBLAS libraries (static and dynamic)" )
else ( )
    message ( STATUS "Building dynamic GraphBLAS library" )
endif ( )

if ( CMAKE_CUDA_DEV )
    # for CUDA development only; not for production use
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DGBCUDA_DEV=1 " )
endif ( )

#-------------------------------------------------------------------------------
# Configure Include/GraphBLAS.h and documentation with version number
#-------------------------------------------------------------------------------

configure_file ( "Config/GraphBLAS.h.in" "${PROJECT_SOURCE_DIR}/Include/GraphBLAS.h" )
configure_file ( "Config/GraphBLAS_version.tex.in" "${PROJECT_SOURCE_DIR}/Doc/GraphBLAS_version.tex" )
configure_file ( "Config/GraphBLAS_API_version.tex.in"
    "${PROJECT_SOURCE_DIR}/Doc/GraphBLAS_API_version.tex" )
configure_file ( "Config/README.md.in" "${PROJECT_SOURCE_DIR}/README.md" )

#-------------------------------------------------------------------------------
# include directories for both graphblas and graphblasdemo libraries
#-------------------------------------------------------------------------------

if ( CMAKE_CUDA_DEV )
    # for CUDA development only; not for production use
    include_directories ( Source/Template Source Include Source/Generated1 lz4 zstd zstd/zstd_subset Demo/Include rmm_wrap )
else ( )
    include_directories ( Source/Template Source Include Source/Generated1 lz4 zstd zstd/zstd_subset Source/Generated2 Demo/Include rmm_wrap )
endif ( )

#-------------------------------------------------------------------------------
# compiler options
#-------------------------------------------------------------------------------

# check which compiler is being used.  If you need to make
# compiler-specific modifications, here is the place to do it.
if ( "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" )
    # The -g option is useful for the Intel VTune tool, but it should be
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
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fno-math-errno " )
    # integer operations wrap
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fwrapv " )
    # check all warnings (uncomment for development only)
#   set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic " )
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
elseif ( "${CMAKE_C_COMPILER_ID}" STREQUAL "IntelLLVM" )
    # options for icx
elseif ( "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" )
    # options for clang
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wno-pointer-sign " )
    if ( CMAKE_C_COMPILER_VERSION VERSION_LESS 3.3 )
        message ( FATAL_ERROR "clang version must be at least 3.3" )
    endif ( )
elseif ( "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC" )
    # options for MicroSoft Visual Studio
    set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2 -wd\"4244\" -wd\"4146\" -wd\"4018\" -wd\"4996\" -wd\"4047\" -wd\"4554\"" )
elseif ( "${CMAKE_C_COMPILER_ID}" STREQUAL "PGI" )
    # options for PGI pgcc compiler.  The compiler has a bug, and the
    # -DPGI_COMPILER_BUG causes GraphBLAS to use a workaround.
    set ( CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -Mnoopenmp -noswitcherror -c11 -lm -DPGI_COMPILER_BUG" )
    set ( CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS} -Mnoopenmp -D__GCC_ATOMIC_TEST_AND_SET_TRUEVAL=1 -noswitcherror --c++11 -lm -DPGI_COMPILER_BUG" )
endif ( )

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}" )
else ( )
    set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}" )
endif ( )

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

if ( CMAKE_CUDA_DEV )
    # for CUDA development only; not for production use
    file ( GLOB GRAPHBLAS_SOURCES "Source/*.c" "Source/Generated1/*.c" )
else ( )
    file ( GLOB GRAPHBLAS_SOURCES "Source/*.c" "Source/Generated1/*.c" "Source/Generated2/*.c" )
endif ( )

add_library ( graphblas SHARED ${GRAPHBLAS_SOURCES} )

SET_TARGET_PROPERTIES ( graphblas PROPERTIES
    VERSION ${GraphBLAS_VERSION_MAJOR}.${GraphBLAS_VERSION_MINOR}.${GraphBLAS_VERSION_SUB}
    SOVERSION ${GraphBLAS_VERSION_MAJOR}
    C_STANDARD_REQUIRED 11
    PUBLIC_HEADER "Include/GraphBLAS.h" )
set_property ( TARGET graphblas PROPERTY C_STANDARD 11 )

if ( SUITESPARSE_CUDA )
    add_dependencies ( graphblas graphblascuda )
    add_dependencies ( graphblas rmm_wrap )
endif ( )

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

    if ( SUITESPARSE_CUDA )
        add_dependencies ( graphblas_static graphblascuda )
        add_dependencies ( graphblas_static rmm_wrap )
    endif ( )
endif ( )

#-------------------------------------------------------------------------------
# cpu_features settings
#-------------------------------------------------------------------------------

if ( NOT GBNCPUFEAT )
    if ( UNIX )
        # look for requirements for cpu_features/src/hwcaps.c
        include ( CheckIncludeFile )
        include ( CheckSymbolExists )
        check_include_file ( dlfcn.h HAVE_DLFCN_H )
        if ( HAVE_DLFCN_H )
            message ( STATUS "cpu_feautures has dlfcn.h" )
            target_compile_definitions ( graphblas PRIVATE HAVE_DLFCN_H )
            if ( BUILD_GRB_STATIC_LIBRARY )
                target_compile_definitions ( graphblas_static PRIVATE HAVE_DLFCN_H )
            endif ( )
        else ( )
            message ( STATUS "cpu_feautures without dlfcn.h" )
        endif ( )
        check_symbol_exists ( getauxval "sys/auxv.h" HAVE_STRONG_GETAUXVAL )
        if ( HAVE_STRONG_GETAUXVAL )
            message ( STATUS "cpu_feautures has getauxval from sys/auxv.h" )
            target_compile_definitions ( graphblas PRIVATE HAVE_STRONG_GETAUXVAL )
            if ( BUILD_GRB_STATIC_LIBRARY )
                target_compile_definitions ( graphblas_static PRIVATE HAVE_STRONG_GETAUXVAL )
            endif ( )
        else ( )
            message ( STATUS "cpu_feautures doesn't have getauxval from sys/auxv.h" )
        endif ( )
    endif ( )
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

if ( OPENMP_FOUND )
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

if ( SUITESPARSE_CUDA )
    target_link_libraries ( graphblas PUBLIC ${GB_CUDA} ${GB_RMM} )
    if ( BUILD_GRB_STATIC_LIBRARY )
        target_link_libraries ( graphblas_static PUBLIC ${GB_CUDA} ${GB_RMM} )
    endif ( )
endif ( )

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
    target_link_libraries ( graphblasdemo PUBLIC ${M_LIB} graphblas  ${GB_CUDA} ${GB_RMM} )

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
    add_executable ( wathen_demo   "Demo/Program/wathen_demo.c" )

    # Libraries required for Demo programs
    target_link_libraries ( openmp_demo   PUBLIC graphblas graphblasdemo ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( complex_demo  PUBLIC graphblas graphblasdemo ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( kron_demo     PUBLIC graphblas graphblasdemo ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( simple_demo   PUBLIC graphblasdemo ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( wildtype_demo PUBLIC graphblas ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( reduce_demo   PUBLIC graphblas ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( import_demo   PUBLIC graphblas graphblasdemo ${GB_CUDA} ${GB_RMM} )
    target_link_libraries ( wathen_demo   PUBLIC graphblas graphblasdemo ${GB_CUDA} ${GB_RMM} )

else ( )

    message ( STATUS "Skipping the demos in GraphBLAS/Demo" )

endif ( )

#-------------------------------------------------------------------------------
# installation location
#-------------------------------------------------------------------------------

# use "cmake -DSUITESPARSE_LOCAL=1" to install only in ../lib and
# ../include.  

if ( NOT SUITESPARSE_LOCAL )
    # install in /usr/local/lib and /usr/local/include.
    # requires "sudo make install"
    message ( STATUS "Installation will be system-wide (requires 'sudo make install')" )
    install ( TARGETS graphblas
        LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )
    if ( BUILD_GRB_STATIC_LIBRARY )
        install ( TARGETS graphblas_static
            ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR} )
    endif ( )
endif ( )

if ( INSIDE_SUITESPARSE )
    # also install in SuiteSparse/lib and SuiteSparse/include;
    # does not require "sudo make install", just "make install"
    message ( STATUS "Installation in ../../lib and ../../include," )
    message ( STATUS "  with 'make local ; make install'. No 'sudo' required." )
    install ( TARGETS graphblas
        LIBRARY       DESTINATION ${SUITESPARSE_LIBDIR}
        PUBLIC_HEADER DESTINATION ${SUITESPARSE_INCLUDEDIR} )
    if ( BUILD_GRB_STATIC_LIBRARY )
        install ( TARGETS graphblas_static
            ARCHIVE       DESTINATION ${SUITESPARSE_LIBDIR} )
    endif ( )
endif ( )

#-------------------------------------------------------------------------------
# dump all variables, for debugging only
#-------------------------------------------------------------------------------

# uncomment this line or add -DDUMP=true to your cmake call to dump all cmake variables:
# set ( DUMP true )

if ( DUMP )
    message("====")
    message("Begin dumping all CMake variables in alphabetic order")
    message("====")
    get_cmake_property ( _variableNames VARIABLES )
    list ( SORT _variableNames )
    foreach ( _variableName ${_variableNames} )
        message ( STATUS "${_variableName}=${${_variableName}}" )
    endforeach ( )
    message("====")
    message("End dumping all CMake variables in alphabetic order")
    message("====")
endif ( )

#-------------------------------------------------------------------------------
# report status
#-------------------------------------------------------------------------------

include ( SuiteSparseReport )

