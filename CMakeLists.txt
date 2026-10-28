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
# Windows Native Host Executable (M2)
# ---------------------------------------------------------------------------
set(GBARECOMP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../gbarecomp-main")
set(GBARECOMP_BUILD "${GBARECOMP_ROOT}/build-vs/Release")
set(SDL2_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../CastlevaniaAriaOfSorrowRecomp/vendor/SDL2")

add_executable(FZeroMaximumVelocityRecomp
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/runtime_shim.cpp"
)

target_include_directories(FZeroMaximumVelocityRecomp PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/framework/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/generated"
    "${GBARECOMP_ROOT}/src/runtime"
    "${GBARECOMP_ROOT}/src/gba"
    "${GBARECOMP_ROOT}/src/armv4t"
    "${GBARECOMP_ROOT}/src/debug"
    "${GBARECOMP_ROOT}/build-vs/_deps/tomlplusplus-src/include"
    "${SDL2_ROOT}/include"
)

set_target_properties(FZeroMaximumVelocityRecomp PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

if(MSVC)
    target_compile_options(FZeroMaximumVelocityRecomp PRIVATE /utf-8 /W3 /EHsc)
    target_link_options(FZeroMaximumVelocityRecomp PRIVATE "/STACK:67108864")
endif()

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
)

add_custom_command(TARGET FZeroMaximumVelocityRecomp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_ROOT}/lib/x64/SDL2.dll"
        "$<TARGET_FILE_DIR:FZeroMaximumVelocityRecomp>/SDL2.dll"
)
