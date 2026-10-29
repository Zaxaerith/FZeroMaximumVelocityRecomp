cmake_minimum_required(VERSION 3.20)
project(gbarecomp_generated C CXX)

file(GLOB GBARECOMP_SHARDS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/recompiled_*.cpp"
)
set(GBARECOMP_GENERATED ${GBARECOMP_SHARDS}
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/dispatch_table.cpp"
)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/symbol_map.cpp")
    list(APPEND GBARECOMP_GENERATED
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/symbol_map.cpp")
endif()

add_library(gbarecomp_game STATIC ${GBARECOMP_GENERATED})
target_include_directories(gbarecomp_game PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/framework/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/generated"
)
set_target_properties(gbarecomp_game PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)
if(EMSCRIPTEN)
    # Generated code needs guaranteed tail calls (see runtime_arm.h).
    target_compile_options(gbarecomp_game PRIVATE -mtail-call)
endif()

# ---------------------------------------------------------------------------
# Dependencies & Paths
# ---------------------------------------------------------------------------
set(GBARECOMP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../gbarecomp-main")
set(GBARECOMP_BUILD "${GBARECOMP_ROOT}/build-vs/Release")
set(SDL2_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../CastlevaniaAriaOfSorrowRecomp/vendor/SDL2")
set(RECOMP_UI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/recomp-ui" CACHE PATH "Root of recomp-ui")

# ---------------------------------------------------------------------------
# recomp-ui Launcher Library (M7)
# ---------------------------------------------------------------------------
option(GBAGAME_RECOMP_UI "Build the recomp-ui launcher" ON)

if(GBAGAME_RECOMP_UI)
    include("${RECOMP_UI_ROOT}/recomp_ui.cmake")

    add_library(game_launcher_ui STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/game_launcher_boot.cpp"
    )

    recomp_target_launcher_ui(game_launcher_ui CONSOLE gba)

    target_include_directories(game_launcher_ui PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/framework/include"
        "${GBARECOMP_ROOT}/src/runtime"
        "${GBARECOMP_ROOT}/src/gba"
        "${GBARECOMP_ROOT}/src/armv4t"
        "${GBARECOMP_ROOT}/src/debug"
        "${GBARECOMP_ROOT}/build-vs/_deps/tomlplusplus-src/include"
        "${SDL2_ROOT}/include"
        "${SDL2_ROOT}/include/SDL2"
        "${RECOMP_UI_ROOT}/src"
        "${RECOMP_UI_ROOT}/src/common"
    )

    target_link_libraries(game_launcher_ui PUBLIC
        "${SDL2_ROOT}/lib/x64/SDL2.lib"
    )

    if(MSVC)
        target_compile_options(game_launcher_ui PRIVATE /utf-8 /W3 /EHsc /Zc:char8_t- /wd4996)
    endif()
endif()

# ---------------------------------------------------------------------------
# Windows Native Host Executable (M2 / M7)
# ---------------------------------------------------------------------------
add_executable(FZeroMaximumVelocityRecomp
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/runtime_shim.cpp"
)

target_include_directories(FZeroMaximumVelocityRecomp PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/framework/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/generated"
    "${GBARECOMP_ROOT}/src/runtime"
    "${GBARECOMP_ROOT}/src/gba"
    "${GBARECOMP_ROOT}/src/armv4t"
    "${GBARECOMP_ROOT}/src/debug"
    "${GBARECOMP_ROOT}/build-vs/_deps/tomlplusplus-src/include"
    "${SDL2_ROOT}/include"
    "${SDL2_ROOT}/include/SDL2"
)

set_target_properties(FZeroMaximumVelocityRecomp PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

if(MSVC)
    target_compile_options(FZeroMaximumVelocityRecomp PRIVATE /utf-8 /W3 /EHsc /wd4996)
    target_link_options(FZeroMaximumVelocityRecomp PRIVATE "/STACK:67108864")
endif()

target_compile_definitions(FZeroMaximumVelocityRecomp PRIVATE
    GBARECOMP_WINDOW_TITLE="F-Zero: Maximum Velocity"
    GBARECOMP_DEFAULT_GAME_CONFIG="game.toml"
)

target_link_libraries(FZeroMaximumVelocityRecomp PRIVATE
    gbarecomp_game
    "${GBARECOMP_BUILD}/gbarecomp_runtime.lib"
    "${GBARECOMP_BUILD}/gbarecomp_debug.lib"
    "${GBARECOMP_BUILD}/gbarecomp_heal_gate.lib"
    "${GBARECOMP_BUILD}/gbarecomp_recompile_core.lib"
    "${GBARECOMP_BUILD}/gbarecomp_gba.lib"
    "${GBARECOMP_BUILD}/gbarecomp_armv4t.lib"
    "${SDL2_ROOT}/lib/x64/SDL2.lib"
    ws2_32
    comdlg32
    user32
    winmm
    shell32
    gdi32
    ole32
    oleaut32
    uuid
    advapi32
    setupapi
    imm32
    version
)

if(GBAGAME_RECOMP_UI)
    target_link_libraries(FZeroMaximumVelocityRecomp PRIVATE
        game_launcher_ui
        opengl32
        d2d1
        dwrite
        windowscodecs
    )
    target_compile_definitions(FZeroMaximumVelocityRecomp PRIVATE
        RECOMP_LAUNCHER
        GBAGAME_RECOMP_UI
    )
    recomp_stage_launcher_assets(FZeroMaximumVelocityRecomp CONSOLE gba)
endif()

add_custom_command(TARGET FZeroMaximumVelocityRecomp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_ROOT}/lib/x64/SDL2.dll"
        "$<TARGET_FILE_DIR:FZeroMaximumVelocityRecomp>/SDL2.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/game.toml"
        "$<TARGET_FILE_DIR:FZeroMaximumVelocityRecomp>/game.toml"
)

# ---------------------------------------------------------------------------
# Launcher Contract Verification Target
# ---------------------------------------------------------------------------
if(GBAGAME_RECOMP_UI)
    add_executable(launcher_contract EXCLUDE_FROM_ALL
        tests/launcher_contract.cpp
    )
    target_compile_definitions(launcher_contract PRIVATE RECOMP_LAUNCHER)
    target_include_directories(launcher_contract PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${GBARECOMP_ROOT}/src/runtime"
        "${GBARECOMP_ROOT}/src/gba"
        "${RECOMP_UI_ROOT}/src"
        "${RECOMP_UI_ROOT}/src/common"
        "${SDL2_ROOT}/include"
        "${SDL2_ROOT}/include/SDL2"
    )
    set_target_properties(launcher_contract PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
    )
    if(MSVC)
        target_compile_options(launcher_contract PRIVATE /utf-8 /W3 /EHsc /Zc:char8_t- /wd4996)
    endif()
    target_link_libraries(launcher_contract PRIVATE
        game_launcher_ui
        "${GBARECOMP_BUILD}/gbarecomp_gba.lib"
        "${SDL2_ROOT}/lib/x64/SDL2.lib"
        opengl32 user32 gdi32 shell32 comdlg32 winmm ws2_32 imm32 version ole32 oleaut32 uuid advapi32 setupapi d2d1 dwrite windowscodecs
    )
endif()
