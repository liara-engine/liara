/**
 * @file main.cpp
 * @brief Phase 0 launcher: hello world + ABI version smoke check.
 */

#include "config.h"

#include "liara/renderer/packet.h"

#include <liara/abi_version.h>
#include <liara/core/core.h>
#include <liara/framework/ModuleLoader.h>
#include <liara/modules.h>
#include <liara/renderer/renderer.h>
#include <liara/result.h>
#include <liara/version.h>

#include <chrono>
#include <cstdint>
#include <format>
#include <initializer_list>
#include <iostream>
#include <string_view>
#include <thread>

/*
 * @brief Minimum ABI version required for this launcher.
 *
 * This constant defines the minimum ABI version required for this launcher to function correctly.
 * It is used to check if the current Liara installation meets the minimum requirements for compatibility.
 *
 * @note This constant needs to be updated to constantly reflect the minimum ABI version required for this launcher.
 */
constexpr uint32_t MIN_ABI_VERSION = LIARA_MAKE_VERSION_UNSAFE(0, 2, 0);
constexpr liara_version_compat_t ABI_COMPAT = liara_version_provides(LIARA_ABI_VERSION, MIN_ABI_VERSION);
static_assert(ABI_COMPAT == LIARA_VERSION_COMPAT_EXACT || ABI_COMPAT == LIARA_VERSION_COMPAT_COMPATIBLE,
              "Liara ABI version is too old for this launcher. Please update your Liara installation.");

constexpr float DEMO_DURATION_SECONDS = 8.0F;
constexpr float TARGET_FRAME_SECONDS = 1.0F / 60.0F;

namespace
{
    /**
     * @brief Check if a list of modules are compatible with the current ABI version.
     * @param modules The list of modules to check.
     * @return True if all modules are compatible, false otherwise.
     */
    bool ModulesAreCompatible(const std::initializer_list<const liara_module_info_t*> modules) {
        bool compatible = true;

        for (const auto* module : modules) {
            if (module == nullptr) {
                std::cout << "Error: Failed to retrieve module information.\n";
                compatible = false;
                continue;
            }

            if (const liara_version_compat_t compat = liara_abi_is_compatible(module->abi_version);
                compat == LIARA_VERSION_COMPAT_EXACT || compat == LIARA_VERSION_COMPAT_COMPATIBLE) {
                std::cout << std::format("{} {} is available and compatible (ABI {}).\n",
                                         module->module_name,
                                         module->module_version_str,
                                         module->abi_version_str);
            }
            else {
                std::cout << std::format("Error: {} {} is not compatible with ABI {}.\n",
                                         module->module_name,
                                         module->module_version_str,
                                         module->abi_version_str);
                compatible = false;
            }
        }
        return compatible;
    }
}  // namespace

int main(int argc, char** argv) {
    const bool smoke = (argc > 1 && std::string_view(argv[1]) == "--smoke");

    std::cout << "Hello from the Liara launcher!\n\n";
    std::cout << std::format("Launcher version: {} (0x{:08x})\n",
                             LIARA_LAUNCHER_VERSION_STRING,
                             LIARA_LAUNCHER_VERSION);

    std::cout << std::format("ABI version:      {} (0x{:08x})\n\n", LIARA_ABI_VERSION_STR, LIARA_ABI_VERSION);

    Liara::Framework::Module<Liara::Framework::CoreApi> core;
    Liara::Framework::Module<Liara::Framework::PlatformApi> platform;
    Liara::Framework::Module<Liara::Framework::RendererApi> renderer;

    for (const Liara::Framework::LoadFailure failure : {core.Load(), platform.Load(), renderer.Load()}) {
        if (failure.Failed()) {
            std::cout << failure.Describe();
            return 1;
        }
    }

#ifdef LIARA_LAUNCHER_MODULE_LOADING_RUNTIME
    std::cout << "Modules resolved at run time.\n";
#else
    std::cout << "Modules linked at build time.\n";
#endif

    if (!ModulesAreCompatible({renderer->info(), core->info(), platform->info()})) {
        std::cout << "\nError: Required modules are not available or compatible. Exiting launcher.\n";
        return 1;
    }

    liara_renderer_handle_t* rendererInstance = nullptr;
    if (renderer->create(&rendererInstance) != LIARA_RESULT_SUCCESS || rendererInstance == nullptr) {
        std::cout << "Error: Failed to create renderer instance.\n";
        return 1;
    }

    liara_core_handle_t* coreInstance = nullptr;
    if (core->create(&coreInstance) != LIARA_RESULT_SUCCESS || coreInstance == nullptr) {
        std::cout << "Error: Failed to create core instance.\n";
        renderer->destroy(rendererInstance);
        return 1;
    }

    if (!smoke) {
        using Clock = std::chrono::steady_clock;

        auto previous = Clock::now();
        float elapsedSeconds = 0.0F;

        while (elapsedSeconds < DEMO_DURATION_SECONDS) {
            const auto frameStart = Clock::now();
            const float deltaTime = std::chrono::duration<float>(frameStart - previous).count();
            previous = frameStart;
            elapsedSeconds += deltaTime;

            core->update(coreInstance, deltaTime);

            if (liara_render_packet_t packet {};
                core->get_render_packet(coreInstance, &packet) == LIARA_RESULT_SUCCESS) {
                renderer->submit_frame(rendererInstance, &packet);
            }

            const auto spent = std::chrono::duration<float>(Clock::now() - frameStart);
            if (const auto remaining = std::chrono::duration<float>(TARGET_FRAME_SECONDS) - spent;
                remaining.count() > 0.0F) {
                std::this_thread::sleep_for(remaining);
            }
        }

        std::cout << "\033[2J\033[H";
        std::cout << std::format("\n{} seconds elapsed. Stopping.\n", DEMO_DURATION_SECONDS);
    }

    core->destroy(coreInstance);
    renderer->destroy(rendererInstance);

    std::cout << "Core finished. Exiting launcher.\n";
    return 0;
}
