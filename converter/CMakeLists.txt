cmake_minimum_required(VERSION 3.12)

project(JAAMConverter)

#Assimp is required for converter only
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_INSTALL OFF CACHE BOOL "")
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "")
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
add_subdirectory(../vendor/src/assimp assimp)

set_property(TARGET zlibstatic PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET assimp PROPERTY FOLDER "ThirdPartyLibraries")

SETUP_APP(JAAMConverter "JAAM")
set_target_properties(JAAMConverter PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "../jaam")

target_link_libraries(JAAMConverter PUBLIC JAAMLib lz4 nlohmann_json assimp)

target_include_directories(JAAMConverter PUBLIC ../lib/include)
target_include_directories(JAAMConverter SYSTEM PRIVATE ../vendor/src/assimp/include)
target_include_directories(JAAMConverter SYSTEM PRIVATE ../vendor/src/stb)
target_include_directories(JAAMConverter SYSTEM PRIVATE ../vendor/src/lz4/lib)
target_include_directories(JAAMConverter SYSTEM PRIVATE ../vendor/src/json/include)
target_include_directories(JAAMConverter SYSTEM PRIVATE ../vendor/src/glm)