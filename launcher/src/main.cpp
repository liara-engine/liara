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
#include <liara/native_window.h>
#include <liara/platform/event.h>
#include <liara/platform/platform.h>
#include <liara/platform/window.h>
#include <liara/renderer/packet.h>
#include <liara/renderer/renderer.h>
#include <liara/result.h>
#include <liara/version.h>

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

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
    constexpr std::string_view USAGE = "Usage: liara_launcher [--smoke] [--gpu <name>] [--frames <count>]\n";

    /** @brief What the command line asked for. */
    struct LaunchOptions
    {
        bool m_Smoke = false;                  ///< Load, negotiate and create the headless instances, then stop.
        std::string m_PreferredGpu;            ///< Passed to the renderer as its preferred device name; empty: none.
        std::optional<uint64_t> m_FrameLimit;  ///< Stop after that many frames, as if the window were closed.
    };

    /** @brief A positive decimal integer, all of `text`, or nothing. */
    std::optional<uint64_t> ParsePositive(const std::string_view text) {
        uint64_t value = 0;
        const char* const end = text.data() + text.size();
        // The range is [text.data(), end): from_chars reads no terminator.
        const auto [stop, error] =
            std::from_chars(text.data(), end, value);  // NOLINT(bugprone-suspicious-stringview-data-usage)
        if (error != std::errc {} || stop != end || value == 0) { return std::nullopt; }
        return value;
    }

    /**
     * @brief Reads the arguments after the program name.
     *
     * @return The options, or nothing when an argument is unknown or a value is missing or invalid, in which case the
     * usage line has been written to `out`.
     */
    std::optional<LaunchOptions> ParseOptions(std::span<char* const> arguments, std::ostream& out) {
        LaunchOptions options;
        while (!arguments.empty()) {
            const std::string_view argument = arguments.front();
            const bool hasValue = arguments.size() > 1;
            const std::string_view value =
                hasValue ? std::string_view(arguments.subspan(1).front()) : std::string_view {};
            if (argument == "--smoke") {
                options.m_Smoke = true;
                arguments = arguments.subspan(1);
                continue;
            }
            if (argument == "--gpu" && hasValue) {
                options.m_PreferredGpu = value;
                arguments = arguments.subspan(2);
                continue;
            }
            if (argument == "--frames" && hasValue) {
                if (const std::optional<uint64_t> count = ParsePositive(value)) {
                    options.m_FrameLimit = count;
                    arguments = arguments.subspan(2);
                    continue;
                }
            }
            out << USAGE;
            return std::nullopt;
        }
        return options;
    }

    /**
     * @brief Runs the frame loop until the platform reports a stop request: the window's close button, Ctrl+C or
     * SIGTERM.
     *
     * Events are polled before the quit flag is read, since a close request reaches the flag through the poll. With a
     * frame limit, the loop also returns once it has submitted that many frames, and the caller shuts down as for a
     * stop request.
     */
    void RunLoop(const Liara::Framework::Module<Liara::Framework::PlatformApi>& platform,
                 liara_platform_handle_t* platformInstance,
                 const Liara::Framework::Module<Liara::Framework::CoreApi>& core,
                 liara_core_handle_t* coreInstance,
                 const Liara::Framework::Module<Liara::Framework::RendererApi>& renderer,
                 liara_renderer_handle_t* rendererInstance,
                 const std::optional<uint64_t> frameLimit) {
        uint64_t previous = platform->time_now_ns();
        uint64_t frames = 0;
        while (true) {
            liara_platform_event_t event {};
            while (platform->poll_event(platformInstance, &event)) {
                // Resizes are not wired to the renderer yet
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

            ++frames;
            if (frameLimit.has_value() && frames >= *frameLimit) { return; }

            platform->time_sleep_until_ns(frameStart + TARGET_FRAME_NS);
        }
    }
}  // namespace

int main(const int argc, char* const* argv) {
    const std::span<char* const> arguments =
        argc > 1 ? std::span<char* const>(argv + 1, static_cast<size_t>(argc - 1)) : std::span<char* const> {};
    const std::optional<LaunchOptions> options = ParseOptions(arguments, std::cout);
    if (!options.has_value()) { return 1; }
    const bool smoke = options->m_Smoke;

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

    liara_core_handle_t* coreInstance = nullptr;
    if (core->create(&coreInstance) != LIARA_RESULT_SUCCESS || coreInstance == nullptr) {
        std::cout << "Error: Failed to create core instance.\n";
        return 1;
    }

    constexpr liara_platform_create_info_t platformInfo {.struct_version = LIARA_PLATFORM_CREATE_INFO_VERSION};
    liara_platform_handle_t* platformInstance = nullptr;
    if (platform->create(&platformInfo, &platformInstance) != LIARA_RESULT_SUCCESS || platformInstance == nullptr) {
        std::cout << "Error: Failed to create platform instance.\n";
        core->destroy(coreInstance);
        return 1;
    }

    if (platform->install_signal_handlers(platformInstance) != LIARA_RESULT_SUCCESS) {
        std::cout << "Error: Failed to install signal handlers.\n";
        platform->destroy(platformInstance);
        core->destroy(coreInstance);
        return 1;
    }

    // --smoke stops here: the modules are loaded and negotiated, and every entry point is resolved, which is what CI
    // can check without a window-system session or a GPU. The window and the renderer need one each.
    if (!smoke) {
        constexpr liara_platform_window_create_info_t windowInfo {
            .struct_version = LIARA_PLATFORM_WINDOW_CREATE_INFO_VERSION,
            .requested_size = WINDOW_SIZE,
            .reserved = 0,
            .title = WINDOW_TITLE.data(),
            .title_length = WINDOW_TITLE.size(),
        };

        liara_platform_window_handle_t* window = nullptr;
        if (platform->window_create(platformInstance, &windowInfo, &window) != LIARA_RESULT_SUCCESS) {
            std::cout
                << "Error: Failed to create the window. Is a graphical session available? --smoke runs without one.\n";
            platform->destroy(platformInstance);
            core->destroy(coreInstance);
            return 1;
        }

        // NOT_SUPPORTED leaves the kind at NONE (SDL's offscreen driver has no native window), which asks the renderer
        // for no surface rather than failing.
        liara_native_window_t native {};
        if (const liara_result_t described = platform->window_get_native(window, &native);
            described != LIARA_RESULT_SUCCESS && described != LIARA_RESULT_NOT_SUPPORTED) {
            std::cout << "Error: Failed to describe the window to the renderer.\n";
            platform->window_destroy(window);
            platform->destroy(platformInstance);
            core->destroy(coreInstance);
            return 1;
        }
        liara_extent2d_t pixelSize {};
        if (platform->window_get_pixel_size(window, &pixelSize) != LIARA_RESULT_SUCCESS) {
            std::cout << "Error: Failed to describe the window to the renderer.\n";
            platform->window_destroy(window);
            platform->destroy(platformInstance);
            core->destroy(coreInstance);
            return 1;
        }

        const liara_renderer_create_info_t rendererInfo {
            .struct_version = LIARA_RENDERER_CREATE_INFO_VERSION,
            .reserved = 0,
            .native_window = native,
            .pixel_size = pixelSize,
            .preferred_device_name = options->m_PreferredGpu.empty() ? nullptr : options->m_PreferredGpu.data(),
            .preferred_device_name_length = options->m_PreferredGpu.size(),
        };

        // Created after the window and destroyed before it: the renderer presents to that window, and a swapchain has
        // to go before the surface it presents to.
        liara_renderer_handle_t* rendererInstance = nullptr;
        if (renderer->create(&rendererInfo, &rendererInstance) != LIARA_RESULT_SUCCESS || rendererInstance == nullptr) {
            std::cout << "Error: Failed to create renderer instance.\n";
            platform->window_destroy(window);
            platform->destroy(platformInstance);
            core->destroy(coreInstance);
            return 1;
        }

        RunLoop(platform, platformInstance, core, coreInstance, renderer, rendererInstance, options->m_FrameLimit);

        renderer->destroy(rendererInstance);
        std::cout << "\nStop requested. Shutting down.\n";
        platform->window_destroy(window);
    }

    platform->destroy(platformInstance);
    core->destroy(coreInstance);

    std::cout << "Core finished. Exiting launcher.\n";
    return 0;
}
