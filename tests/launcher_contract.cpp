#include "launcher_seam.h"
#include "common/launcher_model.h"
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 6) {
        std::fprintf(stderr, "Usage: launcher_contract <valid_rom> <valid_bios> <invalid_rom> <invalid_bios> <scratch_dir>\n");
        return 2;
    }

    auto require = [](bool ok, const char* message) {
        if (!ok) {
            std::fprintf(stderr, "FAIL: %s\n", message);
            std::exit(1);
        }
    };

    RecompLauncherCGameInfo game{};
    launcher_profile_apply("gba", &game);
    const char* sha[] = {"8a08e29ec987f9cbdde21c34d5f7657aa7ba0be6"};
    game.known_sha1_hex = sha;
    game.num_known_sha1 = 1;
    game.bios_verify = &gbarecomp_seam::verify_retail_gba_bios;

    RecompLauncherCSettings settings{};
    settings.window_scale = 3;
    settings.volume = 80;
    std::snprintf(settings.bios_path, sizeof(settings.bios_path), "%s", argv[2]);

    auto model = std::make_unique<LauncherModel>();
    launcher_model_init(model.get(), &settings, &game, argv[1]);

    require(launcher_model_rom_verified(model.get()), "valid F-Zero Maximum Velocity AFZE ROM must verify");
    require(model->setup_bios_ok, "valid retail BIOS must verify");
    require(launcher_model_can_launch(model.get()), "valid inputs must enable launch");

    // Invalid ROM test
    launcher_model_set_rom(model.get(), argv[3]);
    require(!launcher_model_rom_verified(model.get()), "wrong ROM must reject");
    require(!launcher_model_can_launch(model.get()), "wrong ROM must block launch");

    // Recover valid ROM
    launcher_model_set_rom(model.get(), argv[1]);
    require(launcher_model_rom_verified(model.get()), "restoring valid ROM must re-verify");

    // Invalid BIOS test
    launcher_model_set_bios_path(model.get(), argv[4]);
    require(!model->setup_bios_ok, "wrong BIOS must reject");
    require(!launcher_model_can_launch(model.get()), "wrong BIOS must block launch");

    // Missing BIOS test
    launcher_model_set_bios_path(model.get(), "");
    require(!model->setup_bios_ok, "missing retail BIOS must reject");

    // Recover valid BIOS
    launcher_model_set_bios_path(model.get(), argv[2]);
    require(launcher_model_can_launch(model.get()), "valid replacement must recover");

    // Settings commit test
    launcher_model_commit(model.get(), &settings);
    require(settings.volume == 80 && settings.window_scale == 3, "settings must survive commit");

    // Cache roundtrip test
    const std::filesystem::path scratch(argv[5]);
    std::filesystem::create_directories(scratch);
    const auto rom_cache = (scratch / "rom.cfg").string();
    const auto bios_cache = (scratch / "bios.cfg").string();
    gbarecomp_seam::write_single_line(rom_cache, argv[1]);
    gbarecomp_seam::write_single_line(bios_cache, settings.bios_path);
    require(gbarecomp_seam::read_single_line(rom_cache) == argv[1], "ROM cache roundtrip");
    require(std::filesystem::equivalent(gbarecomp_seam::read_single_line(bios_cache), argv[2]), "BIOS cache roundtrip");

    // Settings persistence test in config.ini
    const auto config = (scratch / "config.ini").string();
    std::ofstream(config) << "[Video]\ncustom = preserve\n";
    gbarecomp_seam::SeamConfig saved;
    saved.volume = 80;
    saved.scale = 3;
    saved.skip_launcher = 1;
    gbarecomp_seam::seam_config_save(config, saved);

    gbarecomp_seam::SeamConfig restored;
    gbarecomp_seam::seam_config_load(config, &restored);
    require(restored.volume == 80 && restored.scale == 3 && restored.skip_launcher == 1, "settings persistence");

    std::ifstream input(config);
    std::string contents((std::istreambuf_iterator<char>(input)), {});
    require(contents.find("custom = preserve") != std::string::npos, "preserve unrelated settings");

    std::puts("PASS: launcher ROM/BIOS gates, recovery, caches and settings persistence");
    return 0;
}
