cmake_minimum_required(VERSION 3.29)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../.release-please-manifest.json" RELEASE_PLEASE_MANIFEST)
string(JSON PROJECT_VERSION GET ${RELEASE_PLEASE_MANIFEST} "launcher")
message(STATUS "LiaraLauncher version: ${PROJECT_VERSION}")

project(LiaraLauncher
        VERSION ${PROJECT_VERSION}
        DESCRIPTION "Liara Engine launcher"
        LANGUAGES CXX)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/include/liara/launcher/config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/config.h"
        @ONLY)

if(NOT TARGET Liara::Interfaces)
    message(FATAL_ERROR
            "Liara::Interfaces is not available. Build the launcher through the workspace superbuild "
            " (see https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/BOOTSTRAP)")
endif()

if(NOT TARGET Liara::Renderer)
    message(FATAL_ERROR
            "Liara::Renderer is not available. Build the launcher through the workspace superbuild "
            " (see https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/BOOTSTRAP)")
endif()

if(NOT TARGET Liara::Core)
    message(FATAL_ERROR
            "Liara::Core is not available. Build the launcher through the workspace superbuild "
            " (see https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/BOOTSTRAP)")
endif()

add_executable(liara_launcher "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

target_include_directories(liara_launcher PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_BINARY_DIR}/generated")

target_link_libraries(liara_launcher PRIVATE Liara::Interfaces Liara::Renderer Liara::Core)

set_target_properties(liara_launcher PROPERTIES
        CXX_STANDARD          20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS        OFF)

add_test(NAME launcher_smoke COMMAND liara_launcher)
