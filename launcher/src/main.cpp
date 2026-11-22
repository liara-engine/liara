/**
 * @file main.cpp
 * @brief The reference host: loads the modules, negotiates their ABI versions, opens a window and runs the loop.
 */

#include "config.h"

#include <liara/abi_version.h>
#include <liara/core/core.h>
#include <liara/extent.h>
#include <liara/framework/FixedStep.h>
#include <liara/framework/ModuleLoader.h>
#include <liara/framework/Modules.h>
#include <liara/mesh.h>
#include <liara/native_window.h>
#include <liara/platform/event.h>
#include <liara/platform/platform.h>
#include <liara/platform/window.h>
#include <liara/renderer/packet.h>
#include <liara/renderer/renderer.h>
#include <liara/result.h>
#include <liara/version.h>

#include <array>
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

/** The rate the core is advanced at, whatever the frame rate: every update is one tick of 1/60 s. */
constexpr uint32_t TICKS_PER_SECOND = 60;

/** The loop's fixed step, checked by the compiler: built as a constant, it cannot throw at run time. */
constexpr Liara::Framework::FixedStep FIXED_STEP(TICKS_PER_SECOND);

constexpr std::string_view WINDOW_TITLE = "Liara";
constexpr liara_extent2d_t WINDOW_SIZE {.width = 1280, .height = 720};

// The hello triangle. The launcher holds its vertices in the role liara-assets takes from v0.3: it gives them to the
// renderer under an identifier of its choosing, and tells the core which mesh its packets draw. Normalized device
// coordinates, y up: red at the top, green bottom right, blue bottom left.
constexpr liara_mesh_id_t TRIANGLE_MESH = 1;
constexpr std::array<float, 9> TRIANGLE_POSITIONS {0.0F, 0.5F, 0.0F, 0.5F, -0.5F, 0.0F, -0.5F, -0.5F, 0.0F};
constexpr std::array<uint32_t, 3> TRIANGLE_COLORS {0xFFFF0000U, 0xFF00FF00U, 0xFF0000FFU};

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

    /** @brief What became of a submitted frame. */
    enum class FrameResult : std::uint8_t
    {
        Presented,     ///< An image reached the window.
        NotPresented,  ///< Nothing reached it, and nothing is wrong: no area, no surface, or a swapchain being
                       ///< replaced.
        Failed,        ///< The frame failed, and the loop stops.
    };

    /** @brief Why the loop ended. */
    struct LoopEnd
    {
        liara_result_t m_Failure = LIARA_RESULT_SUCCESS;  ///< SUCCESS for a requested stop or a reached frame limit.
        std::string_view m_What;                          ///< What failed, for the `Error:` line; empty on success.
    };

    /** @brief The last size the platform reported for `window` in one drain of its events, or nothing. */
    std::optional<liara_extent2d_t> DrainEvents(const Liara::Framework::Module<Liara::Framework::PlatformApi>& platform,
                                                liara_platform_handle_t* platformInstance,
                                                const liara_platform_window_handle_t* window) {
        std::optional<liara_extent2d_t> resized;
        liara_platform_event_t event {};
        while (platform->poll_event(platformInstance, &event)) {
            // A drag reports every intermediate size; only the last one of a drain matters to the renderer. `type` says
            // which member of the union is valid, which is how the ABI's event record is read.
            if (event.type == LIARA_PLATFORM_EVENT_WINDOW_RESIZED && event.window == window) {
                resized = event.data.resized.pixel_size;  // NOLINT(cppcoreguidelines-pro-type-union-access)
            }
        }
        return resized;
    }

    /** @brief What became of a frame `submit_frame` answered `result` for. */
    FrameResult ClassifyFrame(const liara_result_t result, const bool rendererPresents) noexcept {
        if (result == LIARA_RESULT_SUCCESS) {
            return rendererPresents ? FrameResult::Presented : FrameResult::NotPresented;
        }
        // The renderer recreates its swapchain on a later frame by itself; there is nothing to do but carry on.
        if (result == LIARA_RESULT_SURFACE_OUT_OF_DATE) { return FrameResult::NotPresented; }
        return FrameResult::Failed;
    }

    /** @brief The `Error:` line's text for a frame that failed with `result`. */
    std::string_view FailedFrameWhat(const liara_result_t result) noexcept {
        switch (result) {
            case LIARA_RESULT_VERSION_MISMATCH:
            case LIARA_RESULT_NULL_POINTER:
            case LIARA_RESULT_INVALID_ARGUMENT: return "The renderer refused the core's render packet";
            default: return "The renderer failed to draw a frame";
        }
    }

    /**
     * @brief Runs the frame loop until the platform reports a stop request (the window's close button, Ctrl+C or
     * SIGTERM), the frame limit is reached, or a frame fails.
     *
     * Each frame drains the events and passes the window's last reported size to the renderer, then reads the quit
     * flag, which a close request reaches through the poll. It then advances the core by as many fixed ticks as the
     * time since the previous frame completes, at most a quarter of a second's worth, extracts one render packet and
     * submits it. A presented frame needs no pacing, since FIFO presentation already waits for the display; a frame
     * that presented nothing sleeps until the next tick is due, so that the loop does not spin while the window has no
     * area.
     *
     * @return Why it ended. A failure carries the code and what failed; the caller reports it and exits with 1.
     */
    LoopEnd RunLoop(const Liara::Framework::Module<Liara::Framework::PlatformApi>& platform,
                    liara_platform_handle_t* platformInstance,
                    const Liara::Framework::Module<Liara::Framework::CoreApi>& core,
                    liara_core_handle_t* coreInstance,
                    const Liara::Framework::Module<Liara::Framework::RendererApi>& renderer,
                    liara_renderer_handle_t* rendererInstance,
                    const liara_platform_window_handle_t* window,
                    const bool rendererPresents,
                    const std::optional<uint64_t> frameLimit) {
        Liara::Framework::FixedStep step = FIXED_STEP;
        uint64_t previous = platform->time_now_ns();
        uint64_t frames = 0;
        while (true) {
            if (const std::optional<liara_extent2d_t> size = DrainEvents(platform, platformInstance, window)) {
                if (const liara_result_t told = renderer->window_resized(rendererInstance, *size);
                    told != LIARA_RESULT_SUCCESS) {
                    return {.m_Failure = told, .m_What = "Failed to tell the renderer the window's size"};
                }
            }
            if (platform->quit_requested(platformInstance)) { return {}; }

            const uint64_t now = platform->time_now_ns();
            const Liara::Framework::FixedStepFrame frame = step.Advance(now - previous);
            previous = now;
            for (uint64_t tick = 0; tick < frame.m_Ticks; ++tick) { core->update(coreInstance, step.StepSeconds()); }

            // frame.m_Alpha, how far the simulation is into the next tick, is not used yet: v0.2 passes it to this
            // extraction, so that what is drawn lies between the last two ticks. This is the only line it touches.
            liara_render_packet_t packet {};
            if (const liara_result_t got = core->get_render_packet(coreInstance, &packet);
                got != LIARA_RESULT_SUCCESS) {
                return {.m_Failure = got, .m_What = "Failed to get the core's render packet"};
            }
            const liara_result_t submitted = renderer->submit_frame(rendererInstance, &packet);
            const FrameResult result = ClassifyFrame(submitted, rendererPresents);
            if (result == FrameResult::Failed) {
                return {.m_Failure = submitted, .m_What = FailedFrameWhat(submitted)};
            }

            ++frames;
            if (frameLimit.has_value() && frames >= *frameLimit) { return {}; }

            if (result == FrameResult::NotPresented) { platform->time_sleep_until_ns(now + step.NsUntilNextTick()); }
        }
    }

    /**
     * @brief Says how the loop ended: an `Error:` line for a failure, the usual line for a stop.
     *
     * @return The launcher's exit code: 1 after a failure, 0 otherwise.
     */
    int ReportLoopEnd(const LoopEnd& end) {
        if (end.m_Failure == LIARA_RESULT_SUCCESS) {
            std::cout << "\nStop requested. Shutting down.\n";
            return 0;
        }
        std::cout << std::format("\nError: {}: {}.\n", end.m_What, liara_result_to_string(end.m_Failure));
        return 1;
    }

    /**
     * @brief Gives the renderer the triangle's mesh and tells the core to draw it.
     *
     * @return Whether both succeeded. On failure, the error has been written to stdout and nothing is left uploaded.
     */
    bool ShowTriangle(const Liara::Framework::Module<Liara::Framework::RendererApi>& renderer,
                      liara_renderer_handle_t* rendererInstance,
                      const Liara::Framework::Module<Liara::Framework::CoreApi>& core,
                      liara_core_handle_t* coreInstance) {
        const liara_mesh_data_t triangle {
            .struct_version = LIARA_MESH_DATA_VERSION,
            .reserved = 0,
            .positions = TRIANGLE_POSITIONS.data(),
            .colors = TRIANGLE_COLORS.data(),
            .vertex_count = TRIANGLE_POSITIONS.size() / 3,
            .indices = nullptr,
            .index_count = 0,
        };
        if (renderer->mesh_upload(rendererInstance, TRIANGLE_MESH, &triangle) != LIARA_RESULT_SUCCESS) {
            std::cout << "Error: Failed to upload the triangle to the renderer.\n";
            return false;
        }
        if (core->draw_mesh(coreInstance, TRIANGLE_MESH) != LIARA_RESULT_SUCCESS) {
            std::cout << "Error: Failed to tell the core which mesh to draw.\n";
            (void)renderer->mesh_release(rendererInstance, TRIANGLE_MESH);
            return false;
        }
        return true;
    }

    /**
     * @brief Creates the renderer, then shows the triangle in it.
     *
     * @return The renderer, or null on failure, with the error written to stdout and nothing of it left behind.
     */
    liara_renderer_handle_t* CreateRendererWithTriangle(
        const Liara::Framework::Module<Liara::Framework::RendererApi>& renderer,
        const liara_renderer_create_info_t& info,
        const Liara::Framework::Module<Liara::Framework::CoreApi>& core,
        liara_core_handle_t* coreInstance) {
        liara_renderer_handle_t* instance = nullptr;
        if (renderer->create(&info, &instance) != LIARA_RESULT_SUCCESS || instance == nullptr) {
            std::cout << "Error: Failed to create renderer instance.\n";
            return nullptr;
        }
        if (!ShowTriangle(renderer, instance, core, coreInstance)) {
            renderer->destroy(instance);
            return nullptr;
        }
        return instance;
    }

    /** @brief Takes the triangle's mesh back from the renderer; a failure is reported and shutdown carries on. */
    void HideTriangle(const Liara::Framework::Module<Liara::Framework::RendererApi>& renderer,
                      liara_renderer_handle_t* rendererInstance) {
        if (renderer->mesh_release(rendererInstance, TRIANGLE_MESH) != LIARA_RESULT_SUCCESS) {
            std::cout << "Warning: Failed to release the triangle's mesh.\n";
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

    int exitCode = 0;

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
        liara_renderer_handle_t* rendererInstance =
            CreateRendererWithTriangle(renderer, rendererInfo, core, coreInstance);
        if (rendererInstance == nullptr) {
            platform->window_destroy(window);
            platform->destroy(platformInstance);
            core->destroy(coreInstance);
            return 1;
        }

        const LoopEnd end = RunLoop(platform,
                                    platformInstance,
                                    core,
                                    coreInstance,
                                    renderer,
                                    rendererInstance,
                                    window,
                                    native.kind != LIARA_NATIVE_WINDOW_KIND_NONE,
                                    options->m_FrameLimit);
        exitCode = ReportLoopEnd(end);

        HideTriangle(renderer, rendererInstance);
        renderer->destroy(rendererInstance);
        platform->window_destroy(window);
    }

    platform->destroy(platformInstance);
    core->destroy(coreInstance);

    std::cout << "Core finished. Exiting launcher.\n";
    return exitCode;
}
