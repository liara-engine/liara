cmake_minimum_required(VERSION 3.29)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
            "In-source builds are not supported. Build into build/<preset-name>/ instead, which is "
            "what the presets already do:\n"
            "    cmake --preset linux-debug-clang\n"
            "Remove the CMakeCache.txt and CMakeFiles/ this attempt just created before retrying.")
endif()

if(NOT TARGET Liara::Interfaces)
    message(FATAL_ERROR
            "Liara::Interfaces is not available. Build through the workspace superbuild "
            "(see https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/bootstrap)")
endif()

set(LIARA_LAUNCHER_MODULE_LOADING "link" CACHE STRING "How a host consumes modules: link | runtime
        link: The host links against the modules at build time. This is the default and recommended option.
        runtime: The host resolves the modules at load time with dlopen/LoadLibrary. Requires them to be built as shared libraries, and is the only mode that can load a module this build system did not build."
)
set_property(CACHE LIARA_LAUNCHER_MODULE_LOADING PROPERTY STRINGS link runtime)

add_library(liara_framework INTERFACE)
add_library(Liara::Framework ALIAS liara_framework)

target_include_directories(liara_framework INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(liara_framework INTERFACE Liara::Interfaces)
target_compile_features(liara_framework INTERFACE cxx_std_20)

if(LIARA_LAUNCHER_MODULE_LOADING STREQUAL "link")
    target_compile_definitions(liara_framework INTERFACE LIARA_LAUNCHER_MODULE_LOADING_LINK)
elseif(LIARA_LAUNCHER_MODULE_LOADING STREQUAL "runtime")
    target_compile_definitions(liara_framework INTERFACE LIARA_LAUNCHER_MODULE_LOADING_RUNTIME)
    target_link_libraries(liara_framework INTERFACE ${CMAKE_DL_LIBS})
    if(NOT BUILD_SHARED_LIBS)
        message(FATAL_ERROR
                "LIARA_LAUNCHER_MODULE_LOADING=runtime requires BUILD_SHARED_LIBS=ON: there is "
                "nothing to load at runtime in a static build. Use a '-runtime' preset.")
    endif()
else()
    message(FATAL_ERROR "Invalid value for LIARA_LAUNCHER_MODULE_LOADING: ${LIARA_LAUNCHER_MODULE_LOADING}. Valid values are: link, runtime")
endif()
