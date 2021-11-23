cmake_minimum_required(VERSION 3.20.1)
project(rmm_wrap VERSION 0.1)

# This build depends upon having RMM cmake installed from https://github.com/rapidsai/rmm.git

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)

find_package(CUDA REQUIRED)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external_includes)
execute_process(
		COMMAND git clone "https://github.com/gabime/spdlog.git" spdlog
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external_includes)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/external_includes/spdlog/include)
execute_process(
		COMMAND git clone "https://github.com/rapidsai/rmm.git" --branch branch-21.08 --recursive rmm
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external_includes)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/external_includes/rmm/include)

add_library(rmm_wrap rmm_wrap.cpp rmm_wrap.hpp rmm_wrap.h)
add_executable(rmm_wrap_test rmm_wrap_test.c rmm_wrap.cpp rmm_wrap.hpp rmm_wrap.h)

set(RMM_WRAP_LIBS ${EXTRA_LIBS} ${CUDA_LIBRARIES})
set(RMM_WRAP_INCLUDES ${PROJECT_BINARY_DIR}
		${CUDA_INCLUDE_DIRS} ${CONDA_PREFIX}/include ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(rmm_wrap PUBLIC ${RMM_WRAP_LIBS})
target_include_directories(rmm_wrap PUBLIC "${RMM_WRAP_INCLUDES}")

target_link_libraries(rmm_wrap_test PUBLIC ${RMM_WRAP_LIBS})
target_include_directories(rmm_wrap_test PUBLIC "${RMM_WRAP_INCLUDES}")
