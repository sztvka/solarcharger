# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)
set(EXTRA_COMPONENT_DIRS ${espidflib_SOURCE_DIR}/lib)

idf_component_register(SRCS ${app_sources})
