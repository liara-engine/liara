cmake_minimum_required(VERSION 3.29)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../.release-please-manifest.json" RELEASE_PLEASE_MANIFEST)
string(JSON PROJECT_VERSION GET ${RELEASE_PLEASE_MANIFEST} "launcher")
message(STATUS "LiaraLauncher version: ${PROJECT_VERSION}")

project(LiaraLauncher
        VERSION ${PROJECT_VERSION}
        DESCRIPTION "Liara Engine launcher"
        LANGUAGES CXX)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/main.cpp"
        @ONLY)

if(NOT TARGET Liara::Interfaces)
    message(FATAL_ERROR
            "Liara::Interfaces is not available. Build the launcher through the "
            "workspace superbuild (see scripts/setup-workspace.sh).")
endif()

add_executable(liara_launcher "${CMAKE_CURRENT_BINARY_DIR}/generated/main.cpp")

target_link_libraries(liara_launcher PRIVATE Liara::Interfaces)

set_target_properties(liara_launcher PROPERTIES
        CXX_STANDARD          20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS        OFF)

add_test(NAME launcher_smoke COMMAND liara_launcher)