/**
* @file main.cpp
 * @brief Phase 0 launcher: hello world + ABI version smoke check.
 */

#include <liara/abi_version.h>
#include <liara/modules.h>

#include <cstdint>
#include <dlfcn.h>
#include <format>
#include <iostream>

#include <liara/core/core.h>
#include <liara/renderer/renderer.h>
#include <liara/version.h>

#include "config.h"
#include "liara/renderer/packet.h"

/*
 * @brief Minimum ABI version required for this launcher.
 *
 * This constant defines the minimum ABI version required for this launcher to function correctly.
 * It is used to check if the current Liara installation meets the minimum requirements for compatibility.
 *
 * @note This constant needs to be updated to constantly reflect the minimum ABI version required for this launcher.
 */
constexpr uint32_t MIN_ABI_VERSION = LIARA_MAKE_VERSION_UNSAFE(0, 2, 0);
static_assert(LIARA_ABI_VERSION >= MIN_ABI_VERSION, "Liara ABI version is too old for this launcher. Please update your Liara installation.");

constexpr float DEMO_DURATION_SECONDS = 8.0F;

int main() {
    std::cout << "Hello from the Liara launcher!\n\n";
    std::cout << std::format("Launcher version: {} (0x{:08x})\n",
                             LIARA_LAUNCHER_VERSION_STRING,
                             LIARA_LAUNCHER_VERSION);

    std::cout << std::format("ABI version:      {} (0x{:08x})\n\n",
                             LIARA_ABI_VERSION_STR,
                             LIARA_ABI_VERSION);

#ifdef LIARA_LAUNCHER_MODULE_LOADING_RUNTIME
    void* coreHandle = dlopen("libliara_core.so", RTLD_LAZY);
    if (coreHandle == nullptr) {
        std::cout << std::format("Error: Failed to load Liara core library ({}).\n", dlerror());
        return 1;
    }

    void* rendererHandle = dlopen("libliara_renderer.so", RTLD_LAZY);
    if (rendererHandle == nullptr) {
        std::cout << std::format("Error: Failed to load Liara renderer library ({}).\n", dlerror());
        dlclose(coreHandle);
        return 1;
    }

    typedef const liara_module_info_t* (*ModuleInfoFunc)();
    const auto liara_core_info = reinterpret_cast<ModuleInfoFunc>(dlsym(coreHandle, "liara_core_info"));
    const auto liara_renderer_info = reinterpret_cast<ModuleInfoFunc>(dlsym(rendererHandle, "liara_renderer_info"));

    if (liara_core_info == nullptr || liara_renderer_info == nullptr) {
        std::cout << std::format("Error: Failed to retrieve module information ({}).\n", dlerror());
        dlclose(coreHandle);
        dlclose(rendererHandle);
        return 1;
    }

    std::cout << "Liara core and renderer libraries loaded successfully.\n";
#endif

    bool error = false;
    for (const auto& module : {liara_renderer_info(), liara_core_info()}) {
        if (module != nullptr) {
            if (liara_version_compat_t const COMPAT = liara_abi_is_compatible(module->abi_version); COMPAT == LIARA_VERSION_COMPAT_EXACT || COMPAT == LIARA_VERSION_COMPAT_COMPATIBLE) {
                std::cout << std::format("{} {} is available and compatible (ABI {}).\n",
                                         module->module_name,
                                         module->module_version_str,
                                         module->abi_version_str);
            } else if (COMPAT == LIARA_VERSION_COMPAT_DEGRADED) {
                std::cout << std::format("Warning: {} {} is degraded with ABI {}. Some features may not work as expected. We highly recommend updating your Liara installation to a compatible version.\n",
                                         module->module_name,
                                         module->module_version_str,
                                         module->abi_version_str);
            } else {
                std::cout << std::format("Error: {} {} is not compatible with ABI {}. Please update your Liara installation to a compatible version.\n",
                                         module->module_name,
                                         module->module_version_str,
                                         module->abi_version_str);
                error = true;
            }
        } else {
            std::cout << "Error: Failed to retrieve module information.\n";
            error = true;
        }
    }

    if (error) {
        std::cout << "\nError: Required modules are not available or compatible. Exiting launcher.\n";
#ifdef LIARA_LAUNCHER_MODULE_LOADING_RUNTIME
        dlclose(coreHandle);
        dlclose(rendererHandle);
#endif
        return 1;
    }

#ifdef LIARA_LAUNCHER_MODULE_LOADING_RUNTIME
    std::cout << "\nDynamic module loading: ABI compatibility smoke test passed.\n";
    dlclose(coreHandle);
    dlclose(rendererHandle);
    return 0;
#else

    liara_renderer_handle_t* renderer = nullptr;
    if (liara_renderer_create(&renderer) != LIARA_RESULT_SUCCESS || renderer == nullptr) {
        std::cout << "Error: Failed to create renderer instance.\n";
        return 1;
    }

    liara_core_handle_t* core = nullptr;
    if (liara_core_create(&core) != LIARA_RESULT_SUCCESS || core == nullptr) {
        std::cout << "Error: Failed to create core instance.\n";
        liara_renderer_destroy(renderer);
        return 1;
    }

    static liara_renderer_handle_t* s_activeRenderer = renderer;

    liara_core_set_late_update_callback(core, [](liara_core_handle_t* lambdaCore, float deltaTime) {
        static float elapsedSeconds = 0.0F;
        elapsedSeconds += deltaTime;

        liara_render_packet_t packet{};
        if (liara_core_get_render_packet(lambdaCore, &packet) == LIARA_RESULT_SUCCESS) {
            liara_renderer_submit_frame(s_activeRenderer, &packet);
        }

        if (elapsedSeconds >= DEMO_DURATION_SECONDS) {
            std::cout << "\033[2J\033[H";
            std::cout << std::format("\n{} seconds elapsed. Stopping core...\n", DEMO_DURATION_SECONDS);
            liara_core_stop(lambdaCore);
        }
    });

    liara_core_set_run_mode(core, LIARA_CORE_RUN_MODE_FIXED, 1.0F / 60.0F);
    liara_core_run(core);

    liara_core_destroy(core);
    liara_renderer_destroy(renderer);

    std::cout << "Core finished. Exiting launcher.\n";
    return 0;
#endif
}
