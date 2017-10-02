cmake_minimum_required(VERSION 3.0.0)
project(graph VERSION 0.0.1)

include(CTest)
enable_testing()

add_subdirectory(src)
add_subdirectory(tests/unit)
add_subdirectory(tests/flow)

add_custom_target(deploydocs mkdocs build
                  COMMAND s3cmd sync site/ s3://redisgraph.io
                  COMMENT "Builds and deploys the module's documentation website"
                  VERBATIM)