/**
 * @file main.cpp
 * @brief Phase 0 launcher: hello world + ABI version smoke check.
 */

#include "config.h"

#include "liara/renderer/packet.h"

#include <liara/abi_version.h>
#include <liara/core/core.h>
#include <liara/framework/ModuleLoader.h>
#include <liara/framework/Modules.h>
#include <liara/platform/platform.h>
#include <liara/renderer/renderer.h>
#include <liara/result.h>
#include <liara/version.h>

#include <cstdint>
#include <format>
#include <initializer_list>
#include <iostream>
#include <string_view>

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

constexpr uint64_t NS_PER_SECOND = 1'000'000'000ULL;
constexpr uint64_t DEMO_DURATION_NS = 8ULL * NS_PER_SECOND;
constexpr uint64_t TARGET_FRAME_NS = NS_PER_SECOND / 60ULL;

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

#ifdef LIARA_MODULE_LOADING_RUNTIME
    std::cout << "Modules resolved at run time.\n";
#else
    std::cout << "Modules linked at build time.\n";
#endif

    if (!Liara::Framework::ModulesAreCompatible({renderer->info(), core->info(), platform->info()}, std::cout)) {
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

    constexpr liara_platform_create_info_t platformInfo {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION};
    liara_platform_handle_t* platformInstance = nullptr;
    if (platform->create(&platformInfo, &platformInstance) != LIARA_RESULT_SUCCESS || platformInstance == nullptr) {
        std::cout << "Error: Failed to create platform instance.\n";
        core->destroy(coreInstance);
        renderer->destroy(rendererInstance);
        return 1;
    }

    if (platform->install_signal_handlers(platformInstance) != LIARA_RESULT_SUCCESS) {
        std::cout << "Error: Failed to install signal handlers.\n";
        platform->destroy(platformInstance);
        core->destroy(coreInstance);
        renderer->destroy(rendererInstance);
        return 1;
    }

    if (!smoke) {
        uint64_t frameStart = platform->time_now_ns();
        const uint64_t start = frameStart;
        uint64_t previous = frameStart;

        while (frameStart - start < DEMO_DURATION_NS && !platform->quit_requested(platformInstance)) {
            const uint64_t deltaNs = frameStart - previous;
            previous = frameStart;

            core->update(coreInstance, static_cast<float>(deltaNs) / static_cast<float>(NS_PER_SECOND));

            if (liara_render_packet_t packet {};
                core->get_render_packet(coreInstance, &packet) == LIARA_RESULT_SUCCESS) {
                renderer->submit_frame(rendererInstance, &packet);
            }

            platform->time_sleep_until_ns(frameStart + TARGET_FRAME_NS);
            frameStart = platform->time_now_ns();
        }

        std::cout << "\033[2J\033[H";
        if (platform->quit_requested(platformInstance)) { std::cout << "\nStop requested. Shutting down.\n"; }
        else { std::cout << std::format("\n{} seconds elapsed. Stopping.\n", DEMO_DURATION_NS / NS_PER_SECOND); }
    }

    platform->destroy(platformInstance);
    core->destroy(coreInstance);
    renderer->destroy(rendererInstance);

    std::cout << "Core finished. Exiting launcher.\n";
    return 0;
}
