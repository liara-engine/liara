/**
 * @file main.cpp
 * @brief The reference host: loads the modules, negotiates their ABI versions, opens a window and runs the loop.
 */

#include "config.h"

#include <liara/abi_version.h>
#include <liara/core/core.h>
#include <liara/extent.h>
#include <liara/framework/ModuleLoader.h>
#include <liara/framework/Modules.h>
#include <liara/platform/event.h>
#include <liara/platform/platform.h>
#include <liara/platform/window.h>
#include <liara/renderer/packet.h>
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
constexpr uint64_t TARGET_FRAME_NS = NS_PER_SECOND / 60ULL;

constexpr std::string_view WINDOW_TITLE = "Liara";
constexpr liara_extent2d_t WINDOW_SIZE {.width = 1280, .height = 720};

namespace
{
    /**
     * @brief Runs the frame loop until the platform reports a stop request: the window's close button, Ctrl+C or
     * SIGTERM.
     *
     * Events are polled before the quit flag is read, since a close request reaches the flag through the poll.
     */
    void RunLoop(const Liara::Framework::Module<Liara::Framework::PlatformApi>& platform,
                 liara_platform_handle_t* platformInstance,
                 const Liara::Framework::Module<Liara::Framework::CoreApi>& core,
                 liara_core_handle_t* coreInstance,
                 const Liara::Framework::Module<Liara::Framework::RendererApi>& renderer,
                 liara_renderer_handle_t* rendererInstance) {
        uint64_t previous = platform->time_now_ns();
        while (true) {
            liara_platform_event_t event {};
            while (platform->poll_event(platformInstance, &event)) {
                // Resizes are not wired to the renderer yet, and a close request needs nothing: quit_requested
                // covers it.
            }
            if (platform->quit_requested(platformInstance)) { return; }

            const uint64_t frameStart = platform->time_now_ns();
            const uint64_t deltaNs = frameStart - previous;
            previous = frameStart;

            core->update(coreInstance, static_cast<float>(deltaNs) / static_cast<float>(NS_PER_SECOND));

            if (liara_render_packet_t packet {};
                core->get_render_packet(coreInstance, &packet) == LIARA_RESULT_SUCCESS) {
                renderer->submit_frame(rendererInstance, &packet);
            }

            platform->time_sleep_until_ns(frameStart + TARGET_FRAME_NS);
        }
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

    // --smoke stops here: modules loaded, versions negotiated, every instance created. It opens no window, so it runs
    // where no window system exists, CI included.
    if (!smoke) {
        const liara_platform_window_create_info_t windowInfo {
            .struct_version = LIARA_PLATFORM_WINDOW_CREATE_INFO_VERSION,
            .requested_size = WINDOW_SIZE,
            .reserved = 0,
            .title = WINDOW_TITLE.data(),
            .title_length = WINDOW_TITLE.size(),
        };
        liara_platform_window_handle_t* window = nullptr;
        if (platform->window_create(platformInstance, &windowInfo, &window) != LIARA_RESULT_SUCCESS) {
            std::cout << "Error: Failed to create the window. Is a graphical session available? "
                         "--smoke runs without one.\n";
            platform->destroy(platformInstance);
            core->destroy(coreInstance);
            renderer->destroy(rendererInstance);
            return 1;
        }

        RunLoop(platform, platformInstance, core, coreInstance, renderer, rendererInstance);

        platform->window_destroy(window);
        std::cout << "\033[2J\033[H" << "\nStop requested. Shutting down.\n";
    }

    platform->destroy(platformInstance);
    core->destroy(coreInstance);
    renderer->destroy(rendererInstance);

    std::cout << "Core finished. Exiting launcher.\n";
    return 0;
}
