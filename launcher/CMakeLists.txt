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

set(LIARA_LAUNCHER_MODULE_LOADING "link" CACHE STRING "How the launcher consumes modules: link | runtime
        link: The launcher links against the modules at build time. This is the default and recommended option.
        runtime (experimental): The launcher loads the modules at runtime. This option is only available if the modules are built as shared libraries."
)
set_property(CACHE LIARA_LAUNCHER_MODULE_LOADING PROPERTY STRINGS link runtime)

if(LIARA_LAUNCHER_MODULE_LOADING STREQUAL "link")
    add_compile_definitions(LIARA_LAUNCHER_MODULE_LOADING_LINK)
elseif(LIARA_LAUNCHER_MODULE_LOADING STREQUAL "runtime")
    add_compile_definitions(LIARA_LAUNCHER_MODULE_LOADING_RUNTIME)
else()
    message(FATAL_ERROR "Invalid value for LIARA_LAUNCHER_MODULE_LOADING: ${LIARA_LAUNCHER_MODULE_LOADING}. Valid values are: link, runtime")
endif()


if(NOT TARGET Liara::Interfaces)
    message(FATAL_ERROR
            "Liara::Interfaces is not available. Build the launcher through the workspace superbuild "
            " (see https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/BOOTSTRAP)")
endif()

if(NOT TARGET Liara::Core)
    find_package(LiaraCore REQUIRED)
    if(NOT TARGET Liara::Core)
        message(FATAL_ERROR "Liara::Core is not available. Build the launcher through the workspace superbuild "
                " (see https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/BOOTSTRAP)")
    endif()
endif()

if(NOT TARGET Liara::Renderer)
    find_package(LiaraRenderer REQUIRED)
    if(NOT TARGET Liara::Renderer)
        message(FATAL_ERROR "Liara::Renderer is not available. Build the launcher through the workspace superbuild "
                " (see https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/BOOTSTRAP)")
    endif()
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
