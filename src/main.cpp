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

std::filesystem::path get_executable_dir(char* argv0) {
    std::error_code ec;
#ifdef _WIN32
    wchar_t exe_path[MAX_PATH];
    const DWORD n = GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
    if (n > 0 && n < MAX_PATH) {
        return std::filesystem::path(exe_path).parent_path();
    }
#endif
    auto p = std::filesystem::absolute(argv0, ec);
    if (!ec) return p.parent_path();
    return std::filesystem::current_path();
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
    options.max_view_width = 240;
    options.freely_resizable_window = true;

    const auto exe_dir = get_executable_dir(argv[0]);
    auto project_root = exe_dir;
    if (std::filesystem::exists(exe_dir.parent_path() / "gbarecomp-project.json")) {
        project_root = exe_dir.parent_path();
    } else if (std::filesystem::exists(exe_dir.parent_path().parent_path() / "gbarecomp-project.json")) {
        project_root = exe_dir.parent_path().parent_path();
    }

    std::vector<std::string> args;
    args.reserve(argc + 8);
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    auto has_arg = [&](const std::string& key) {
        for (const auto& a : args) {
            if (a == key) return true;
        }
        return false;
    };

    // Locate ROM
    if (!has_arg("--rom")) {
        std::filesystem::path candidate_roms[] = {
            project_root / "F-Zero - Maximum Velocity (USA, Europe).gba",
            exe_dir / "F-Zero - Maximum Velocity (USA, Europe).gba",
            std::filesystem::current_path() / "F-Zero - Maximum Velocity (USA, Europe).gba"
        };
        for (const auto& cand : candidate_roms) {
            if (std::filesystem::exists(cand)) {
                args.emplace_back("--rom");
                args.emplace_back(std::filesystem::canonical(cand).string());
                break;
            }
        }
    }

    // Locate BIOS
    if (!has_arg("--bios")) {
        std::filesystem::path candidate_bios[] = {
            project_root / "gba_bios.bin",
            exe_dir / "gba_bios.bin",
            std::filesystem::path("D:/MYAPPLICA/1GAMAMA/GBAGAMES/gbabios/gba_bios.bin")
        };
        for (const auto& cand : candidate_bios) {
            if (std::filesystem::exists(cand)) {
                args.emplace_back("--bios");
                args.emplace_back(std::filesystem::canonical(cand).string());
                break;
            }
        }
    }

    // Setup saves directory
    if (!has_arg("--save")) {
        std::error_code ec;
        const auto saves_dir = project_root / "saves";
        std::filesystem::create_directories(saves_dir, ec);
        args.emplace_back("--save");
        args.emplace_back((saves_dir / "fzero_maximum_velocity.sav").string());
    }

    // Default to windowed mode
    if (!has_arg("--window") && !has_arg("--fullscreen")) {
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
