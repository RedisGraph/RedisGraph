cmake_minimum_required ( VERSION 3.19 )

# CMake build for generating googletest c++ files that can be compiled and executed in parallel.
# Build can be customized to speed up development by allowing the targeting of specific
# specific parameters. The output of this build is an executable that can be used to
# run the gtests.

project(GRAPHBLAS_CUDA VERSION 0.1 LANGUAGES CXX CUDA)

set(CMAKE_CUDA_FLAGS "-cudart=static -lineinfo ")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17 -fPIC ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGBNCPUFEAT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGBNCPUFEAT")
set(CMAKE_C_STANDARD 11)

message(STATUS "C++ flags for CUDA:" "${CMAKE_CXX_FLAGS}")

file(GLOB GRAPHBLAS_CUDA_SOURCES "*.cu" "*.c" "*.cpp")

add_library(graphblascuda SHARED
            ${GRAPHBLAS_CUDA_SOURCES}
)

set(RMM_WRAP_INCLUDES "../rmm_wrap")

message(STATUS "RMM_WRAP_INCLUDES: ${RMM_WRAP_INCLUDES}")
set(GRAPHBLAS_CUDA_INCLUDES
        ${RMM_WRAP_INCLUDES}
        ../Source
        ../Source/Template
        ../Include
        ../CUDA)

message(STATUS "GraphBLAS CUDA includes: " "${GRAPHBLAS_CUDA_INCLUDES}")

set(EXTERNAL_INCLUDES_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external_includes)

IF(NOT EXISTS ${EXTERNAL_INCLUDES_DIRECTORY})
    file(MAKE_DIRECTORY ${EXTERNAL_INCLUDES_DIRECTORY})
endif()

IF(NOT EXISTS ${EXTERNAL_INCLUDES_DIRECTORY}/cuco)
    execute_process(
            COMMAND git clone "https://github.com/NVIDIA/cuCollections.git" --branch main --recursive cuco
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external_includes)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/external_includes/cuco/include)

target_include_directories(graphblascuda PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/external_includes/cuco/include
        ${CUDAToolkit_INCLUDE_DIRS}
        ${GRAPHBLAS_CUDA_INCLUDES})
set_target_properties(graphblascuda PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(graphblascuda PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
set_target_properties(graphblascuda PROPERTIES CUDA_ARCHITECTURES "75")

target_link_libraries(graphblascuda CUDA::nvrtc CUDA::cudart_static CUDA::nvToolsExt )

#-------------------------------------------------------------------------------
# installation location
#-------------------------------------------------------------------------------

# use "cmake -DSUITESPARSE_LOCAL=1" to install only in ../lib and
# ../include.  

if ( NOT SUITESPARSE_LOCAL )
    # install in /usr/local/lib and /usr/local/include.
    # requires "sudo make install"
    message ( STATUS "Installation will be system-wide (requires 'sudo make install')" )
    install ( TARGETS graphblascuda
        LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )
#   if ( BUILD_GRB_STATIC_LIBRARY )
#       install ( TARGETS graphblas_static
#           ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR} )
#   endif ( )
endif ( )

if ( INSIDE_SUITESPARSE )
    # also install in SuiteSparse/lib and SuiteSparse/include;
    # does not require "sudo make install", just "make install"
    message ( STATUS "Installation in ../../lib and ../../include," )
    message ( STATUS "  with 'make local ; make install'. No 'sudo' required." )
    install ( TARGETS graphblascuda
        LIBRARY       DESTINATION ${SUITESPARSE_LIBDIR}
        PUBLIC_HEADER DESTINATION ${SUITESPARSE_INCLUDEDIR} )
#   if ( BUILD_GRB_STATIC_LIBRARY )
#       install ( TARGETS graphblas_static
#           ARCHIVE       DESTINATION ${SUITESPARSE_LIBDIR} )
#   endif ( )
endif ( )

# install ( TARGETS graphblascuda
#         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
#         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR} )

#-------------------------------------------------------------------------------

# 1. Execute enumify/stringify/jitify logic to compile ptx kernels and compile/link w/ relevant *.cu files.

# TODO: Need to do this piece in cmake

# 2. Generate test .cu files named "{semiring_operation}_test_instances.hpp"
set(CUDA_TEST_SUITES
    AxB_dot3
#    reduce_to_scalar
)

#
set(CUDA_TEST_MONOIDS PLUS MIN MAX) # TIMES ANY)
set(CUDA_TEST_BINOPS TIMES PLUS MIN MAX DIV) #MINUS RDIV RMINUS FIRST SECOND PAIR)
set(CUDA_TEST_SEMIRINGS PLUS_TIMES MIN_PLUS MAX_PLUS)
set(CUDA_TEST_DATATYPES int32_t int64_t uint32_t uint64_t float double)
set(CUDA_TEST_KERNELS vsvs) # mp vsvs dndn spdn vssp)
set(CUDA_TEST_FORMATS sparse dense sparse_dense reduce)


# TODO: Update testGen.py to accept the above CUDA_TEST_* params as arguments

# Note: I don't believe there's a way to do this particular piece in parallel but
# once all the files are written, we should be able to compile them in parallel

# Separate individual kernels from larger "overview" test (e.g. 2-level testing structure)
# We want to test all the *_cuda versions

# FIXME
set(CUDA_TEST_CPP_FILES "")
if ( FALSE )
    foreach(var ${CUDA_TEST_SUITES})
        foreach(semiring ${CUDA_TEST_SEMIRINGS})
            foreach(kernel ${CUDA_TEST_KERNELS})
                foreach(format ${CUDA_TEST_FORMATS})
                    # TODO: Have Python script also build separate cudaTest.cpp (named something
                    # like AxB_dot3_cuda_tests.cpp) for each suite. This way we should be able to
                    # easily ignore them from the build
                    add_custom_command(
                            OUTPUT
                            ${CMAKE_CURRENT_BINARY_DIR}/${var}_${semiring}_${format}_test_instances.hpp
                            ${CMAKE_CURRENT_BINARY_DIR}/${var}_${semiring}_${format}_cuda_tests.cpp
                            DEPENDS
                            jitFactory.hpp
                            COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test/testGen_cmake.py "\"${CMAKE_CURRENT_SOURCE_DIR}\"" "\"${var}\"" "\"${CUDA_TEST_MONOIDS}\""
                                "\"${CUDA_TEST_BINOPS}\"" "\"${semiring}\"" "\"${CUDA_TEST_DATATYPES}\""
                                "\"${kernel}\""
                    )
                    # Construct final list of files to compile (in parallel)
                    list(APPEND CUDA_TEST_CPP_FILES ${CMAKE_CURRENT_BINARY_DIR}/${var}_${semiring}_${format}_cuda_tests.cpp)
                endforeach()
            endforeach()
        endforeach()
    endforeach()
endif ( )

include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

#FetchContent_MakeAvailable(googletest EC)


#file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external_includes)
#execute_process(
#        COMMAND git clone "https://github.com/google/googletest.git" googletest
#        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external_includes)
#
#include_directories(${CMAKE_CURRENT_BINARY_DIR}/external_includes/googletest/googletest/include)

#add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/external_includes/googletest/googletest/)

# 3. Compile/link individual {test_suite_name}_cuda_tests.cpp files into a gtest executable
set(GRAPHBLAS_CUDA_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/test)

message(STATUS "CUDA tests files: " "${CUDA_TEST_CPP_FILES}")

add_executable(graphblascuda_test ${CUDA_TEST_CPP_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/test/run_tests.cpp)

set_target_properties(graphblascuda_test PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(graphblascuda_test PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
set_target_properties(graphblascuda_test PROPERTIES CUDA_ARCHITECTURES "75")

include(GoogleTest)

add_dependencies(graphblascuda_test graphblas)
add_dependencies(graphblascuda_test graphblascuda)
add_dependencies(graphblascuda_test gtest_main)
add_dependencies(graphblascuda_test rmm_wrap)

target_link_libraries(graphblascuda_test
        PUBLIC
        graphblas
        graphblascuda
        CUDA::cudart_static
        CUDA::nvrtc
        ${ADDITIONAL_DEPS}
        PRIVATE
        gtest_main)

target_include_directories(graphblascuda_test
        PUBLIC
        ${ADDITIONAL_INCLUDES}
        ${CUDAToolkit_INCLUDE_DIRS}
        ${GRAPHBLAS_CUDA_INCLUDES})

