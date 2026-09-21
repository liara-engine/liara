/**
 * @file ModuleLoader.h
 */

#pragma once

#include <liara/abi_version.h>
#include <liara/core/core_functions.h>
#include <liara/modules.h>
#include <liara/platform/platform_functions.h>
#include <liara/renderer/renderer_functions.h>
#include <liara/version.h>

#include <cstdint>
#include <format>
#include <string>

// NOLINTBEGIN(cppcoreguidelines-macro-usage, bugprone-macro-parentheses)

#ifdef _WIN32
    #define LIARA_LIB_NAME(stem) stem ".dll"
#else
    #define LIARA_LIB_NAME(stem) "lib" stem ".so"
#endif

#ifdef LIARA_LAUNCHER_MODULE_LOADING_RUNTIME
    #ifdef _WIN32
        #include <windows.h>
using LibHandle = HMODULE;
        #define LIARA_LIB_LOAD(path)           LoadLibraryA(path)
        #define LIARA_LIB_SYMBOL(handle, name) GetProcAddress(handle, name)
        #define LIARA_LIB_FREE(handle)         FreeLibrary(handle)
        #define LIARA_LIB_ERROR()              std::to_string(GetLastError())
    #else
        #include <dlfcn.h>
using LibHandle = void*;
        #define LIARA_LIB_LOAD(path)           dlopen(path, RTLD_LAZY)
        #define LIARA_LIB_SYMBOL(handle, name) dlsym(handle, name)
        #define LIARA_LIB_FREE(handle)         dlclose(handle)
        #define LIARA_LIB_ERROR()              std::string(dlerror())
    #endif
#endif

#define LIARA_MODULE_MEMBER(prefix, suffix, returnType, parameters)    returnType(*suffix) parameters = nullptr;
#define LIARA_MODULE_LINK_FILL(prefix, suffix, returnType, parameters) api.suffix = &prefix##_##suffix;

#define LIARA_MODULE_RUNTIME_FILL(prefix, suffix, returnType, parameters)                                    \
    api.suffix = reinterpret_cast<returnType(*) parameters>(LIARA_LIB_SYMBOL(library, #prefix "_" #suffix)); \
    if (api.suffix == nullptr) { return {LoadFailure::Reason::SymbolMissing, #prefix "_" #suffix}; }

#ifdef LIARA_LAUNCHER_MODULE_LOADING_RUNTIME
    #define LIARA_DEFINE_MODULE_API(Name, prefix, FUNCTION_LIST)                                                       \
        namespace Liara::Launcher                                                                                      \
        {                                                                                                              \
            struct Name##Api                                                                                           \
            {                                                                                                          \
                static constexpr const char* LIBRARY_FILE = LIARA_LIB_NAME(#prefix);                                   \
                FUNCTION_LIST(LIARA_MODULE_MEMBER, prefix)                                                             \
            };                                                                                                         \
            inline LoadFailure LoadApi(Name##Api& api, LibHandle library) {                                            \
                api.info =                                                                                             \
                    reinterpret_cast<const liara_module_info_t* (*)()>(LIARA_LIB_SYMBOL(library, #prefix "_info"));    \
                if (api.info == nullptr) { return {LoadFailure::Reason::SymbolMissing, #prefix "_info"}; }             \
                const liara_module_info_t* moduleInfo = api.info();                                                    \
                if (moduleInfo == nullptr) { return {LoadFailure::Reason::InfoUnavailable, #prefix "_info"}; }         \
                if (moduleInfo->struct_version < LIARA_MODULE_INFO_VERSION) {                                          \
                    return {LoadFailure::Reason::UnsupportedInfoVersion, #prefix "_info", moduleInfo->struct_version}; \
                }                                                                                                      \
                FUNCTION_LIST(LIARA_MODULE_RUNTIME_FILL, prefix)                                                       \
                return {};                                                                                             \
            }                                                                                                          \
        }
#else
    #define LIARA_DEFINE_MODULE_API(Name, prefix, FUNCTION_LIST)                     \
        namespace Liara::Launcher                                                    \
        {                                                                            \
            struct Name##Api                                                         \
            {                                                                        \
                static constexpr const char* LIBRARY_FILE = LIARA_LIB_NAME(#prefix); \
                FUNCTION_LIST(LIARA_MODULE_MEMBER, prefix)                           \
            };                                                                       \
            inline LoadFailure LoadApi(Name##Api& api) {                             \
                FUNCTION_LIST(LIARA_MODULE_LINK_FILL, prefix)                        \
                return {};                                                           \
            }                                                                        \
        }
#endif

// NOLINTEND(cppcoreguidelines-macro-usage, bugprone-macro-parentheses)

namespace Liara::Launcher
{
    /**
     * @brief The platform's description of the last dynamic-loading failure.
     *
     * Defined in both modes so that a caller reporting a LoadFailure needs no `#ifdef` of its own.
     * Under the link presets nothing is ever opened, so there is nothing for it to describe.
     *
     * @return A string describing the last dynamic-loading failure.
     */
    inline std::string LastLibraryError() {
#ifdef LIARA_LAUNCHER_MODULE_LOADING_RUNTIME
        return LIARA_LIB_ERROR();
#else
        return "this build links its modules rather than opening them";
#endif
    }

    /**
     * @brief Why a module could not be loaded. A `Reason::None` means it loaded.
     */
    struct LoadFailure
    {
        enum class Reason : uint8_t
        {
            None,
            LibraryNotFound,
            InfoUnavailable,
            UnsupportedInfoVersion,
            SymbolMissing,
        };

        Reason m_Reason = Reason::None;
        const char* m_Detail = nullptr;
        uint32_t m_Value = 0;  ///< The `struct_version` that was not understood, for UnsupportedInfoVersion.

        /**
         * @brief Check if the load failed.
         * @return True if the load failed, false otherwise.
         */
        [[nodiscard]] bool Failed() const { return m_Reason != Reason::None; }

        /**
         * @brief Describe the load failure.
         * @return A string describing the load failure.
         */
        [[nodiscard]] std::string Describe() const {
            switch (m_Reason) {
                using enum Reason;
                case LibraryNotFound:
                    return std::format("Error: could not open {} ({}).\n", m_Detail, LastLibraryError());
                case InfoUnavailable: return std::format("Error: {} returned no module info.\n", m_Detail);
                case UnsupportedInfoVersion:
                    return std::format(
                        "Error: {} reports module info version {}, older than the {} this launcher reads.\n",
                        m_Detail,
                        m_Value,
                        LIARA_MODULE_INFO_VERSION);
                case SymbolMissing: return std::format("Error: a module does not export {}.\n", m_Detail);
                case None: break;
            }
            return {};
        }
    };

    /**
     * @brief Owns one module's dispatch table, and under the runtime presets the library it came from.
     *
     * Under the link presets there is no library to own and the object degrades to the table alone.
     */
    template<typename Api>
    class Module
    {
    public:
        Module() = default;

        /**
         * @brief Destroy the module's library and clear its entry points.
         */
        ~Module() {
#ifdef LIARA_LAUNCHER_MODULE_LOADING_RUNTIME
            if (m_Library != nullptr) { LIARA_LIB_FREE(m_Library); }
#endif
        }

        Module(const Module&) = delete;
        Module& operator=(const Module&) = delete;
        Module(Module&&) = delete;
        Module& operator=(Module&&) = delete;

        /**
         * @brief Load the module's library and resolve its entry points.
         * @return A LoadFailure describing any failure, or Reason::None if successful.
         */
        [[nodiscard]] LoadFailure Load() {
#ifdef LIARA_LAUNCHER_MODULE_LOADING_RUNTIME
            m_Library = LIARA_LIB_LOAD(Api::LIBRARY_FILE);
            if (m_Library == nullptr) { return {LoadFailure::Reason::LibraryNotFound, Api::LIBRARY_FILE}; }
            return LoadApi(m_Api, m_Library);
#else
            return LoadApi(m_Api);
#endif
        }

        const Api* operator->() const { return &m_Api; }

    private:
        Api m_Api {};
#ifdef LIARA_LAUNCHER_MODULE_LOADING_RUNTIME
        LibHandle m_Library = nullptr;
#endif
    };
}  // namespace Liara::Launcher

// NOLINTBEGIN(readability-identifier-naming)
LIARA_DEFINE_MODULE_API(Core, liara_core, LIARA_CORE_FUNCTIONS)
LIARA_DEFINE_MODULE_API(Platform, liara_platform, LIARA_PLATFORM_FUNCTIONS)
LIARA_DEFINE_MODULE_API(Renderer, liara_renderer, LIARA_RENDERER_FUNCTIONS)
// NOLINTEND(readability-identifier-naming)
