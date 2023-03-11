cmake_minimum_required(VERSION 3.13)

get_filename_component(root ${CMAKE_CURRENT_LIST_DIR} ABSOLUTE)
include(${root}/deps/readies/cmake/main)

SET(CMAKE_EXPORT_COMPILE_COMMANDS on)

if (NOT DEFINED UNIT_TESTS)
	set(UNIT_TESTS OFF)
endif()

project(redisgraph)

setup_cc_options()

file(GLOB_RECURSE SOURCES "src/*.c")

add_library(redisgraph SHARED ${SOURCES})

include_directories(
	${root}/src
	${root}
	${root}/deps
	${root}/deps/rax
	${root}/deps/xxHash
	${root}/deps/utf8proc
	${root}/deps/oniguruma
	${root}/deps/RediSearch/src
	${root}/deps/GraphBLAS/Include
	${root}/deps/libcypher-parser/lib/src
	$ENV{LIBCYPHER_PARSER_BINDIR}/lib/src)

add_compile_definitions(
	_GNU_SOURCE
	REDIS_MODULE_TARGET
	REDISMODULE_EXPERIMENTAL_API
	XXH_STATIC_LINKING_ONLY)

if (DEBUG)
	add_compile_definitions(RG_DEBUG)
endif()

if (MEMCHECK)
	add_compile_definitions(MEMCHECK)
endif()

setup_shared_object_target(redisgraph "")

set_target_properties(redisgraph PROPERTIES LINKER_LANGUAGE CXX)

set(REDISGRAPH_OBJECTS $<TARGET_OBJECTS:redisgraph>)

lists_from_env(GRAPHBLAS LIBXXHASH RAX LIBCYPHER_PARSER REDISEARCH_LIBS UTF8PROC ONIGURUMA)
set(REDISGRAPH_LIBS ${GRAPHBLAS} ${LIBXXHASH} ${RAX} ${LIBCYPHER_PARSER} ${REDISEARCH_LIBS} ${UTF8PROC} ${ONIGURUMA})

target_link_options(redisgraph PRIVATE ${CMAKE_LD_FLAGS_LIST} ${CMAKE_SO_LD_FLAGS_LIST})
target_link_libraries(redisgraph PRIVATE ${REDISGRAPH_LIBS} ${CMAKE_LD_LIBS})

extract_debug_symbols(redisgraph)

if (APPLE)
	add_library(redisgraph_static STATIC $<TARGET_OBJECTS:redisgraph>)
endif()

if (UNIT_TESTS)
	add_subdirectory(${root}/tests/unit tests/unit)
endif()

