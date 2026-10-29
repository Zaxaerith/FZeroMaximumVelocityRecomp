// F-Zero: Maximum Velocity (USA, Europe, AFZE) — launcher seam & input validation.
#include "game_launcher_boot.h"
#include "launcher_seam.h"
#include "gba_bios.h"
#include "common/sha1.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

int game_launcher_preboot(std::vector<std::string>& args, const gbarecomp::RunOptions& options) {
    return gbarecomp_launcher_preboot(args, options);
}

int game_validate_inputs(std::vector<std::string>& args, const gbarecomp::RunOptions& options) {
    std::string path;
    std::string rom_path;

    for (std::size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--help" || args[i] == "-h") return 0;
        if (args[i] == "--bios" && i + 1 < args.size()) path = args[++i];
        else if (args[i] == "--rom" && i + 1 < args.size()) rom_path = args[++i];
    }

    const auto exe_dir = std::filesystem::path(gbarecomp_seam::exe_dir(args));

    // BIOS discovery & validation
    if (path.empty()) {
        path = gbarecomp_seam::read_single_line((exe_dir / "bios.cfg").string());
    }
    if (path.empty()) {
        const std::filesystem::path candidate_bios[] = {
            exe_dir / "gba_bios.bin",
            exe_dir.parent_path() / "gba_bios.bin",
            exe_dir.parent_path().parent_path() / "gba_bios.bin",
            "gba_bios.bin",
            "D:/MYAPPLICA/1GAMAMA/GBAGAMES/gbabios/gba_bios.bin"
        };
        for (const auto& cand : candidate_bios) {
            std::error_code ec;
            if (std::filesystem::exists(cand, ec) && !ec) {
                path = std::filesystem::absolute(cand, ec).string();
                break;
            }
        }
    }
    if (path.empty()) {
        std::fprintf(stderr, "[fzero] Retail GBA BIOS required: select it in the launcher or pass --bios.\n");
        return 1;
    }

    gba::GbaBios bios;
    std::string error;
    if (!bios.load_from_file(path, gba::GbaBios::kExpectedSha1, &error)) {
        std::fprintf(stderr, "[fzero] BIOS identity rejected: %s\n", error.c_str());
        return 1;
    }

    // ROM discovery & validation
    if (rom_path.empty()) {
        rom_path = gbarecomp_seam::read_single_line((exe_dir / "rom.cfg").string());
    }
    if (rom_path.empty()) {
        const std::filesystem::path candidate_roms[] = {
            exe_dir / "F-Zero - Maximum Velocity (USA, Europe).gba",
            exe_dir.parent_path() / "F-Zero - Maximum Velocity (USA, Europe).gba",
            exe_dir.parent_path().parent_path() / "F-Zero - Maximum Velocity (USA, Europe).gba",
            "F-Zero - Maximum Velocity (USA, Europe).gba"
        };
        for (const auto& cand : candidate_roms) {
            std::error_code ec;
            if (std::filesystem::exists(cand, ec) && !ec) {
                rom_path = std::filesystem::absolute(cand, ec).string();
                break;
            }
        }
    }
    if (rom_path.empty()) {
        std::fprintf(stderr, "[fzero] ROM required: select it in the launcher or pass --rom.\n");
        return 1;
    }

    std::ifstream rom(rom_path, std::ios::binary | std::ios::ate);
    constexpr std::size_t kFZeroRomSize = 4194304; // 4 MiB
    if (!rom || static_cast<std::size_t>(rom.tellg()) != kFZeroRomSize) {
        std::fprintf(stderr, "[fzero] Expected 4 MiB ROM (4,194,304 bytes); select it in the launcher or pass --rom.\n");
        return 1;
    }

    std::vector<std::uint8_t> bytes(kFZeroRomSize);
    rom.seekg(0);
    rom.read(reinterpret_cast<char*>(bytes.data()), bytes.size());
    if (!rom || bytes.size() < 0xAC + 4) {
        std::fprintf(stderr, "[fzero] Failed reading ROM header.\n");
        return 1;
    }

    // Verify game code 'AFZE' at offset 0xAC
    if (std::memcmp(&bytes[0xAC], "AFZE", 4) != 0) {
        std::fprintf(stderr, "[fzero] Game code mismatch at 0xAC; expected 'AFZE'.\n");
        return 1;
    }

    // Verify SHA-1 hash using recomp-ui bundled sha1
    uint8_t digest[20] = {0};
    recompui_sha1_compute(bytes.data(), bytes.size(), digest);
    char computed_sha1[41] = {0};
    recompui_sha1_hex(digest, computed_sha1);

    auto to_lower = [](std::string s) {
        for (char& c : s) if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + 32);
        return s;
    };
    if (options.builtin_rom_sha1 && to_lower(computed_sha1) != to_lower(options.builtin_rom_sha1)) {
        std::fprintf(stderr, "[fzero] ROM SHA-1 mismatch; expected %s, got %s.\n",
                     options.builtin_rom_sha1, computed_sha1);
        return 1;
    }

    // Pass validated inputs to runtime
    auto has_flag = [&](const std::string& flag) {
        for (const auto& a : args) if (a == flag) return true;
        return false;
    };

    if (!has_flag("--rom")) {
        args.emplace_back("--rom");
        args.emplace_back(rom_path);
    }
    if (!has_flag("--bios")) {
        args.emplace_back("--bios");
        args.emplace_back(path);
    }
    if (!has_flag("--rom-sha1") && options.builtin_rom_sha1) {
        args.emplace_back("--rom-sha1");
        args.emplace_back(options.builtin_rom_sha1);
    }
    if (!has_flag("--save")) {
        const char* save_name = options.launcher_save_path ? options.launcher_save_path : "saves/fzero_maximum_velocity.sav";
        std::filesystem::path save_p(save_name);
        if (save_p.is_relative()) {
            save_p = exe_dir / save_p;
        }
        std::error_code ec;
        std::filesystem::create_directories(save_p.parent_path(), ec);
        args.emplace_back("--save");
        args.emplace_back(save_p.string());
    }

    return 0;
}
