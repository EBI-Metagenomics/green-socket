cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)

project(
  gs
  VERSION 0.3.0
  LANGUAGES C)

# Generate compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CPM.cmake)
include(cmake/compiler-options.cmake)
include(cmake/sanitizers.cmake)
include(cmake/libev.cmake)

cpmaddpackage(NAME c_toolbelt VERSION 0.3.1 GITHUB_REPOSITORY horta/c-toolbelt)
cpmaddpackage(NAME cco VERSION 0.0.9 GITHUB_REPOSITORY
              EBI-Metagenomics/c-containers)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(gs src/gs.c src/ctx.c src/task.c src/die.c src/loop.c src/sync.c)
add_library(GS::gs ALIAS gs)
target_link_libraries(gs PRIVATE Threads::Threads)
target_link_libraries(gs PUBLIC C_TOOLBELT::c_toolbelt)
target_link_libraries(gs PUBLIC CCO::cco)
target_link_libraries(gs PUBLIC ev)

set(EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/gs/export.h)
include(GenerateExportHeader)
generate_export_header(
  gs
  BASE_NAME
  GS
  INCLUDE_GUARD_NAME
  GS_EXPORT_H
  EXPORT_MACRO_NAME
  GS_API
  EXPORT_FILE_NAME
  ${EXPORT_FILE})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  include(CheckIPOSupported)
  check_ipo_supported()
  set_target_properties(gs PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_compile_features(gs PRIVATE c_std_11)
target_compile_options(gs PRIVATE ${WARNING_FLAGS})

target_include_directories(
  gs
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

install(TARGETS gs EXPORT gs-targets)

install(DIRECTORY include/gs/ DESTINATION include/gs)

install(
  EXPORT gs-targets
  FILE gs-targets.cmake
  NAMESPACE GS::
  DESTINATION lib/cmake/gs)

include(CMakePackageConfigHelpers)

set(CMAKE_CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/gs-config.cmake)
configure_package_config_file(gs-config.cmake.in ${CMAKE_CONFIG_FILE}
                              INSTALL_DESTINATION lib/cmake/gs)

install(FILES ${CMAKE_CONFIG_FILE} DESTINATION lib/cmake/gs)
install(FILES ${EXPORT_FILE} ${CONFIG_FILE} DESTINATION include/gs)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/README.md
              ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE DESTINATION share/docs/gs)

enable_testing()
add_subdirectory(test)

set(CPACK_PACKAGE_NAME gs)
set(CPACK_PACKAGE_VENDOR "Danilo Horta")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "C library for non-blocking socket communication")
set(CPACK_PACKAGE_CONTACT "Danilo Horta")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/package)
set(CPACK_VERBATIM_VARIABLES YES)
set(CPACK_PACKAGE_RELOCATABLE YES)
set(CPACK_MONOLITHIC_INSTALL YES)
include(CPack)
