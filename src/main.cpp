#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "runtime.h"
#include "game_launcher_boot.h"

namespace {

void suppress_diagnostics() {
#ifdef _WIN32
    if (!std::getenv("GBARECOMP_SELFHEAL_RECOMPILE"))
        _putenv_s("GBARECOMP_SELFHEAL_RECOMPILE", "0");
    if (!std::getenv("GBARECOMP_MISS_FRAG"))
        _putenv_s("GBARECOMP_MISS_FRAG", "NUL");
    if (!std::getenv("GBARECOMP_COVERAGE_JSON"))
        _putenv_s("GBARECOMP_COVERAGE_JSON", "NUL");
    if (!std::getenv("GBARECOMP_SESSION_DIAGNOSTICS"))
        _putenv_s("GBARECOMP_SESSION_DIAGNOSTICS", "0");
    if (!std::getenv("GBARECOMP_HANG_WATCHDOG"))
        _putenv_s("GBARECOMP_HANG_WATCHDOG", "0");
#endif
}

}  // namespace

int main(int argc, char** argv) {
    std::printf("=========================================\n");
    std::printf("  F-Zero: Maximum Velocity Recomp (MSVC)\n");
    std::printf("=========================================\n");
    std::fflush(stdout);

    suppress_diagnostics();

    gbarecomp::RunOptions options;
    options.builtin_game_name = "F-Zero: Maximum Velocity";
    options.builtin_rom_sha1  = "8a08e29ec987f9cbdde21c34d5f7657aa7ba0be6";
    options.builtin_rom_crc32 = 0xbd5e9798u;
    options.launcher_region   = "USA, Europe";
    options.launcher_game_config = "game.toml";
    options.launcher_save_path = "saves/fzero_maximum_velocity.sav";
    options.max_view_width = 240;
    options.freely_resizable_window = true;
    options.launcher_expose_sharp_filter = true;
    options.launcher_default_sharp_filter = true;
    options.launcher_expose_affine_filter = false;
    options.launcher_expose_widescreen = false;
    options.launcher_expose_adaptive_view = false;
    options.expose_assist_tools = true;

    std::vector<std::string> args;
    args.reserve(argc + 12);
    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::puts("FZeroMaximumVelocityRecomp [--rom <path>] [--bios <path>] [--launcher] [--no-launcher] [--help]");
            return 0;
        }
        args.emplace_back(argv[i]);
    }

    // Preboot launcher: opens recomp-ui GUI unless skipped by explicit CLI / headless / skip_launcher
    if (game_launcher_preboot(args, options) != 0) {
        // User quit launcher window without booting
        return 0;
    }

    // Validate inputs (ROM integrity, retail BIOS, default flags)
    if (game_validate_inputs(args, options) != 0) {
        return 1;
    }

    auto has_arg = [&](const std::string& key) {
        for (const auto& a : args) {
            if (a == key) return true;
        }
        return false;
    };

    // Default to windowed mode if neither window, fullscreen, nor no-window was passed
    if (!has_arg("--window") && !has_arg("--fullscreen") && !has_arg("--no-window")) {
        args.emplace_back("--window");
    }

    std::printf("[host] Launching with %zu arguments:\n", args.size());
    for (std::size_t i = 0; i < args.size(); ++i) {
        std::printf("  [%zu] %s\n", i, args[i].c_str());
    }
    std::fflush(stdout);

    std::vector<char*> native_args;
    native_args.reserve(args.size());
    for (auto& a : args) {
        native_args.push_back(a.data());
    }

    int rc = -1;
    try {
        rc = gbarecomp::run_game(static_cast<int>(native_args.size()),
                                 native_args.data(), options);
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "[host] Exception in run_game: %s\n", ex.what());
        std::fflush(stderr);
    } catch (...) {
        std::fprintf(stderr, "[host] Unknown exception in run_game\n");
        std::fflush(stderr);
    }

    std::printf("[host] run_game returned: %d\n", rc);
    std::fflush(stdout);
    return rc;
}
