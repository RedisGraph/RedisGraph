cmake_minimum_required(VERSION 3.13)

get_filename_component(root ${CMAKE_CURRENT_LIST_DIR}/.. ABSOLUTE)
include(${root}/deps/readies/mk/cmake/main)

project(redisgraph)

file(GLOB_RECURSE SOURCES "*.c")

add_library(redisgraph SHARED ${SOURCES})

set_property(TARGET redisgraph PROPERTY C_STANDARD 11)

include_directories(
	${root}/src
	${root}
	${root}/deps
	${root}/deps/rax
	${root}/deps/xxHash
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
	add_compile_definitions(_DEBUG RG_DEBUG)
endif()

list_from_env(CMAKE_CC_FLAGS)
target_compile_options(redisgraph PUBLIC -fPIC ${CMAKE_CC_FLAGS})

list_from_env(LIBOMP)
add_link_options(${LIBOMP})
target_link_options(redisgraph PUBLIC ${LIBOMP})

set_target_properties(redisgraph PROPERTIES PREFIX "")
set_target_properties(redisgraph PROPERTIES SUFFIX ".so")

set_target_properties(redisgraph PROPERTIES LINKER_LANGUAGE CXX)

set(REDISGRAPH_OBJECTS $<TARGET_OBJECTS:redisgraph>)

list_from_env(REDISEARCH_LIBS)
set(REDISGRAPH_LIBS $ENV{GRAPHBLAS} $ENV{LIBXXHASH} $ENV{RAX} $ENV{LIBCYPHER_PARSER} ${REDISEARCH_LIBS})

target_link_libraries(redisgraph PRIVATE ${REDISGRAPH_LIBS})
target_link_libraries(redisgraph PRIVATE c m dl pthread)

add_library(redisgraph_static STATIC $<TARGET_OBJECTS:redisgraph>)

add_subdirectory(${root}/tests/unit tests/unit)
