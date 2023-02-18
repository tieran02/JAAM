cmake_minimum_required(VERSION 3.12)

project(JAAMLib)

SETUP_LIB(JAAMLib "JAAM")

target_include_directories(JAAMLib PRIVATE include)

target_compile_options(JAAMLib PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /wd4100>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

# assimp settings
#set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
#set(ASSIMP_INSTALL OFF CACHE BOOL "")
#set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "")
#set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "")
#set(BUILD_SHARED_LIBS ON CACHE BOOL "")
#add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/src/assimp)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/src/json nlohmann_json)

add_library(lz4 STATIC)
target_sources(lz4 PRIVATE 
    "${CMAKE_SOURCE_DIR}/vendor/src/lz4/lib/lz4.h"
    "${CMAKE_SOURCE_DIR}/vendor/src/lz4/lib/lz4.c"
)

target_include_directories(JAAMLib SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor/src/lz4/lib)
target_include_directories(JAAMLib SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor/src/json/include)
target_include_directories(JAAMLib SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor/src/magic_enum/include)

target_link_libraries(JAAMLib PRIVATE lz4 nlohmann_json)

set_property(TARGET lz4 PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET nlohmann_json PROPERTY FOLDER "ThirdPartyLibraries")