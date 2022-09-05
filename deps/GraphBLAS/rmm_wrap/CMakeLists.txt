#-------------------------------------------------------------------------------
# rmm_wrap/CMakeLists.txt: cmake script for building rmm_wrap
#-------------------------------------------------------------------------------

# SPDX-License-Identifier: Apache-2.0

#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20.1)
project(rmm_wrap VERSION 0.1)

# This build depends upon having RMM cmake installed from https://github.com/rapidsai/rmm.git

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")


find_package(CUDA REQUIRED)

set(EXTERNAL_INCLUDES_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external_includes)

IF(NOT EXISTS ${EXTERNAL_INCLUDES_DIRECTORY})
file(MAKE_DIRECTORY ${EXTERNAL_INCLUDES_DIRECTORY})
endif()

IF(NOT EXISTS ${EXTERNAL_INCLUDES_DIRECTORY}/spdlog)
execute_process(
		COMMAND git clone "https://github.com/gabime/spdlog.git" spdlog
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external_includes)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/external_includes/spdlog/include)

IF(NOT EXISTS ${EXTERNAL_INCLUDES_DIRECTORY}/rmm)
execute_process(
		COMMAND git clone "https://github.com/rapidsai/rmm.git" --branch branch-21.08 --recursive rmm
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external_includes)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/external_includes/rmm/include)

add_library(rmm_wrap rmm_wrap.cpp rmm_wrap.hpp rmm_wrap.h)

SET_TARGET_PROPERTIES (rmm_wrap PROPERTIES
		VERSION ${GraphBLAS_VERSION_MAJOR}.${GraphBLAS_VERSION_MINOR}.${GraphBLAS_VERSION_SUB}
		SOVERSION ${GraphBLAS_VERSION_MAJOR}
		C_STANDARD_REQUIRED 11
		PUBLIC_HEADER "rmm_wrap.h" )


add_executable(rmm_wrap_test rmm_wrap_test.c rmm_wrap.cpp rmm_wrap.hpp rmm_wrap.h)

set(RMM_WRAP_LIBS ${EXTRA_LIBS} ${CUDA_LIBRARIES})
set(RMM_WRAP_INCLUDES ${PROJECT_BINARY_DIR}
		${CUDA_INCLUDE_DIRS} ${CONDA_PREFIX}/include ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(rmm_wrap PUBLIC ${RMM_WRAP_LIBS})
target_include_directories(rmm_wrap PUBLIC "${RMM_WRAP_INCLUDES}")

target_link_libraries(rmm_wrap_test PUBLIC ${RMM_WRAP_LIBS})
target_include_directories(rmm_wrap_test PUBLIC "${RMM_WRAP_INCLUDES}")

install (TARGETS rmm_wrap
		PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR} )
