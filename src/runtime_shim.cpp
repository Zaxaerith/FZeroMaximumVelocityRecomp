#include <cstdint>
#include <cstring>
#include "runtime_arm.h"
#include "runtime_wait_inline.h"

// 1. Thread-local RAM base
GBARECOMP_TAIL_ARG_TLS uint32_t g_runtime_native_ram_base = 0u;

// 2. Runtime wait states table (Power-on GBA bus defaults)
RuntimeWaitTable g_runtime_waits = {
    {4, 2, 2, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0},
    {2, 2, 2, 0, 0, 0, 0, 0, 2, 2, 4, 4, 8, 8, 4, 0},
    {4, 2, 2, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0},
    {2, 2, 2, 0, 0, 0, 0, 0, 2, 2, 4, 4, 8, 8, 4, 0},
    0, // prefetch
    0  // last_prefetched_pc
};

// 3. Native RAM template registration
static const RuntimeNativeRamTemplate* s_templates = nullptr;
static uint32_t s_template_count = 0;
static const RuntimeNativeRamEntry* s_entries = nullptr;
static uint32_t s_entry_count = 0;

extern "C" void runtime_register_native_ram_templates(
    const RuntimeNativeRamTemplate* templates, uint32_t template_count,
    const RuntimeNativeRamEntry* entries, uint32_t entry_count) {
    s_templates = templates;
    s_template_count = template_count;
    s_entries = entries;
    s_entry_count = entry_count;
}

// 4. Prefetch stall delta calculation
extern "C" uint32_t runtime_prefetch_stall_delta(uint32_t wait, uint32_t pc, uint32_t thumb) {
    const int32_t w = static_cast<int32_t>(wait);
    return static_cast<uint32_t>(rwt_prefetch_stall(&g_runtime_waits, w, pc, thumb) - w);
}

// 5. Exception return refill cycles
static unsigned mode_to_bank_shim(uint32_t mode) {
    switch (mode & 0x1Fu) {
        case 0x11u: return ARM_BANK_FIQ;
        case 0x12u: return ARM_BANK_IRQ;
        case 0x13u: return ARM_BANK_SUPERVISOR;
        case 0x17u: return ARM_BANK_ABORT;
        case 0x1Bu: return ARM_BANK_UNDEFINED;
        default:    return ARM_BANK_USER;
    }
}

extern "C" uint32_t runtime_exception_return_refill(uint32_t target_pc) {
    const uint32_t mode = g_cpu.cpsr & 0x1Fu;
    uint32_t psr = g_cpu.cpsr;
    if (mode != 0x10u && mode != 0x1Fu) {
        psr = g_cpu.banked_spsr[mode_to_bank_shim(g_cpu.cpsr)];
    }
    return runtime_refill_cycles(target_pc & ~1u, (psr & CPSR_T_BIT) ? 1u : 0u);
}

// 6 & 7. Instruction matching in native RAM
extern "C" uint32_t bus_read_u32(uint32_t addr);
extern "C" uint16_t bus_read_u16(uint32_t addr);

extern "C" int runtime_native_ram_instruction_matches_masked(uint32_t pc,
    uint32_t opcode, uint32_t size, uint32_t mask) {
    uint32_t actual = 0;
    if (size == 4) {
        actual = bus_read_u32(pc);
    } else if (size == 2) {
        actual = bus_read_u16(pc);
    } else {
        return 0;
    }
    return ((actual ^ opcode) & mask) == 0 ? 1 : 0;
}

extern "C" int runtime_native_ram_instruction_matches(uint32_t pc,
    uint32_t opcode, uint32_t size) {
    return runtime_native_ram_instruction_matches_masked(pc, opcode, size, 0xFFFFFFFFu);
}

// 8. Immediate extraction for native ARM (Official GBARecomp ARM immediate rotate decoder)
extern "C" uint32_t runtime_native_arm_immediate(uint32_t pc) {
    uint32_t insn = bus_read_u32(pc);
    const uint32_t value = insn & 0xFFu, rotate = ((insn >> 8) & 15u) * 2u;
    return rotate ? (value >> rotate) | (value << (32u - rotate)) : value;
}
