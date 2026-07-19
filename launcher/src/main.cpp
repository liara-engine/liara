/**
* @file main.cpp
 * @brief Phase 0 launcher: hello world + ABI version smoke check.
 */

#include <liara/abi_version.h>

#include <cstdint>
#include <format>
#include <iostream>

#include <liara/core/core.h>
#include <liara/renderer/renderer.h>
#include <liara/version.h>

#include "config.h"


int main() {
    bool rendererAvailable = false;
    bool coreAvailable = false;

    std::cout << "Hello from the Liara launcher!\n\n";
    std::cout << std::format("Launcher version: {}.{}.{} (0x{:08x})\n",
                             LIARA_VERSION_MAJOR(LIARA_LAUNCHER_VERSION),
                             LIARA_VERSION_MINOR(LIARA_LAUNCHER_VERSION),
                             LIARA_VERSION_PATCH(LIARA_LAUNCHER_VERSION),
                             LIARA_LAUNCHER_VERSION);

    std::cout << std::format("ABI version:      {}.{}.{} (0x{:08x})\n\n",
                             static_cast<uint32_t>(LIARA_ABI_VERSION_MAJOR),
                             static_cast<uint32_t>(LIARA_ABI_VERSION_MINOR),
                             static_cast<uint32_t>(LIARA_ABI_VERSION_PATCH),
                             LIARA_ABI_VERSION);

    if (liara_abi_version_satisfies(LIARA_MAKE_VERSION_UNSAFE(0, 0, 1))) {
        std::cout << "Warning: ABI 0.0.1 is supported, but not recommended. Please consider updating to a newer version.\n";
        std::cout << "         You may experience limited functionality.\n";
    } else if (liara_abi_version_satisfies(LIARA_MAKE_VERSION_UNSAFE(0, 1, 0))) {
        std::cout << "You use the recommended ABI version 0.1.0. All features are supported.\n";

        if (liara_version_satisfies(liara_renderer_abi_version(), LIARA_MAKE_VERSION_UNSAFE(0, 1, 0))) {
            rendererAvailable = true;
            std::cout << std::format("Renderer {}.{}.{} is available and compatible with ABI 0.1.0.\n",
                                     static_cast<uint32_t>(LIARA_VERSION_MAJOR(liara_renderer_version())),
                                     static_cast<uint32_t>(LIARA_VERSION_MINOR(liara_renderer_version())),
                                     static_cast<uint32_t>(LIARA_VERSION_PATCH(liara_renderer_version())));
        } else {
            std::cout << std::format("Error: Renderer {}.{}.{} is not compatible with ABI 0.1.0. Please update your Liara installation to a compatible version.\n",
                                     static_cast<uint32_t>(LIARA_VERSION_MAJOR(liara_renderer_version())),
                                     static_cast<uint32_t>(LIARA_VERSION_MINOR(liara_renderer_version())),
                                     static_cast<uint32_t>(LIARA_VERSION_PATCH(liara_renderer_version())));
            return 1;
        }

        if (liara_version_satisfies(liara_core_abi_version(), LIARA_MAKE_VERSION_UNSAFE(0, 1, 0))) {
            coreAvailable = true;
            std::cout << std::format("Core {}.{}.{} is available and compatible with ABI 0.1.0.\n",
                                     static_cast<uint32_t>(LIARA_VERSION_MAJOR(liara_core_version())),
                                     static_cast<uint32_t>(LIARA_VERSION_MINOR(liara_core_version())),
                                     static_cast<uint32_t>(LIARA_VERSION_PATCH(liara_core_version())));
        } else {
            std::cout << std::format("Error: Core {}.{}.{} is not compatible with ABI 0.1.0. Please update your Liara installation to a compatible version.\n",
                                     static_cast<uint32_t>(LIARA_VERSION_MAJOR(liara_core_version())),
                                     static_cast<uint32_t>(LIARA_VERSION_MINOR(liara_core_version())),
                                     static_cast<uint32_t>(LIARA_VERSION_PATCH(liara_core_version())));
            return 1;
        }

    } else {
        std::cout << std::format("Error: ABI version {}.{}.{} is not supported. Please update your Liara installation to a compatible version.\n",
                                 static_cast<uint32_t>(LIARA_VERSION_MAJOR(LIARA_ABI_VERSION)),
                                 static_cast<uint32_t>(LIARA_VERSION_MINOR(LIARA_ABI_VERSION)),
                                 static_cast<uint32_t>(LIARA_VERSION_PATCH(LIARA_ABI_VERSION)));
        return 1;
    }

    if (coreAvailable && rendererAvailable)
    {
        std::cout << "\nLaunching core...\n";

        liara_renderer_handle_t* renderer = nullptr;
        liara_renderer_create(&renderer);
        if (renderer == nullptr) {
            std::cout << "Error: Failed to create renderer instance.\n";
            return 1;
        }

        liara_core_handle_t* core = nullptr;
        liara_core_create(renderer, &core);
        if (core == nullptr) {
            std::cout << "Error: Failed to create core instance.\n";
            liara_renderer_destroy(renderer);
            return 1;
        }

        // Set a late update callback to stop the core after 5 seconds
        liara_core_set_late_update_callback(core, [](liara_core_handle_t* lambdaCore, float deltaTime) {
            static float elapsedTime = 0.0F;
            elapsedTime += deltaTime;
            if (elapsedTime >= 5.0F) {
                std::cout << "5 seconds elapsed. Stopping core...\n";
                liara_core_stop(lambdaCore);
            }
        });

        liara_core_set_run_mode(core, LIARA_CORE_RUN_MODE_FIXED, 0.016F);
        liara_core_run(core);

        liara_core_destroy(core);
        liara_renderer_destroy(renderer);

        std::cout << "Core finished. Exiting launcher.\n";
    }

    return 0;
}
