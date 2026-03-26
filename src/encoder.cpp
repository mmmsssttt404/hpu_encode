#include "hpu/encoder.hpp"

#include <stdexcept>

namespace hpu {
namespace {

constexpr std::uint32_t kCustom0Opcode = 0b0001011;
constexpr std::uint32_t kCustom1Opcode = 0b0101011;

struct Custom0Spec {
    std::uint32_t class_code;
    std::uint32_t op_code;
    Format format;
};

Custom0Spec lookup_custom0_spec(Mnemonic mnemonic) {
    switch (mnemonic) {
        case Mnemonic::kPadd:    return {0b000, 0b000, Format::kRRR};
        case Mnemonic::kPsub:    return {0b000, 0b001, Format::kRRR};
        case Mnemonic::kPmul:    return {0b000, 0b010, Format::kRRR};
        case Mnemonic::kPmac:    return {0b000, 0b011, Format::kRRR};
        case Mnemonic::kPmov:    return {0b000, 0b100, Format::kRRR};
        case Mnemonic::kPbcast:  return {0b000, 0b101, Format::kCFG};
        case Mnemonic::kPntt:    return {0b001, 0b000, Format::kRRR};
        case Mnemonic::kPintt:   return {0b001, 0b001, Format::kRRR};
        case Mnemonic::kPtwld:   return {0b010, 0b000, Format::kCFG};
        case Mnemonic::kPtwid:   return {0b010, 0b001, Format::kCFG};
        case Mnemonic::kPtwi2:   return {0b010, 0b010, Format::kCFG};
        case Mnemonic::kPshcfg:  return {0b011, 0b000, Format::kCFG};
        case Mnemonic::kPshuf:   return {0b011, 0b001, Format::kCFG};
        case Mnemonic::kPshuf2:  return {0b011, 0b010, Format::kRRR};
        case Mnemonic::kPseed:   return {0b100, 0b000, Format::kCFG};
        case Mnemonic::kPsample: return {0b100, 0b001, Format::kCFG};
        case Mnemonic::kPmodld:  return {0b101, 0b000, Format::kCFG};
        case Mnemonic::kPmodsw:  return {0b101, 0b001, Format::kCFG};
        case Mnemonic::kSload:   return {0b110, 0b00,  Format::kMEM};
        case Mnemonic::kSstore:  return {0b110, 0b01,  Format::kMEM};
        default:
            throw std::runtime_error("instruction does not belong to custom0");
    }
}

void ensure_range(int value, int min_value, int max_value, const char* field_name) {
    if (value < min_value || value > max_value) {
        throw std::runtime_error(std::string(field_name) + " is out of range");
    }
}

std::uint32_t encode_custom0_rrr(const Instruction& instruction, const Custom0Spec& spec) {
    ensure_range(instruction.prd, 0, 15, "prd");
    ensure_range(instruction.prs1, 0, 15, "prs1");

    const int prs2 = instruction.mnemonic == Mnemonic::kPmov ? 0 : instruction.prs2;
    ensure_range(prs2, 0, 15, "prs2");

    std::uint32_t flag = instruction.flag;
    if (instruction.mnemonic == Mnemonic::kPshuf2 && instruction.pshf >= 0) {
        ensure_range(instruction.pshf, 0, 3, "pshf");
        flag |= static_cast<std::uint32_t>(instruction.pshf);
    }
    if (flag > 0x7F) {
        throw std::runtime_error("FLAG is out of range");
    }

    std::uint32_t word = 0;
    word |= spec.class_code << 29;
    word |= spec.op_code << 26;
    word |= static_cast<std::uint32_t>(instruction.prd) << 22;
    word |= static_cast<std::uint32_t>(instruction.prs1) << 18;
    word |= static_cast<std::uint32_t>(prs2) << 14;
    word |= flag << 7;
    word |= kCustom0Opcode;
    return word;
}

std::uint32_t encode_custom0_cfg(const Instruction& instruction, const Custom0Spec& spec) {
    std::uint32_t idx0 = 0;
    std::uint32_t idx1 = 0;
    std::uint32_t cfg = instruction.cfg;

    switch (instruction.mnemonic) {
        case Mnemonic::kPbcast:
            ensure_range(instruction.prd, 0, 15, "prd");
            ensure_range(instruction.pcst, 0, 3, "pcst");
            idx0 = static_cast<std::uint32_t>(instruction.prd);
            idx1 = static_cast<std::uint32_t>(instruction.pcst);
            break;
        case Mnemonic::kPtwld:
            ensure_range(instruction.ptw, 0, 15, "ptw");
            idx0 = static_cast<std::uint32_t>(instruction.ptw);
            break;
        case Mnemonic::kPtwid:
        case Mnemonic::kPtwi2:
            break;
        case Mnemonic::kPshcfg:
            ensure_range(instruction.pshf, 0, 0x1FF, "pshf");
            idx0 = static_cast<std::uint32_t>(instruction.pshf & 0xF);
            cfg |= static_cast<std::uint32_t>(instruction.pshf >> 4);
            break;
        case Mnemonic::kPshuf:
            ensure_range(instruction.prd, 0, 15, "prd");
            ensure_range(instruction.prs1, 0, 15, "prs1");
            idx0 = static_cast<std::uint32_t>(instruction.prd);
            idx1 = static_cast<std::uint32_t>(instruction.prs1);
            if (instruction.pshf >= 0) {
                ensure_range(instruction.pshf, 0, 3, "pshf");
                cfg |= static_cast<std::uint32_t>(instruction.pshf) << 7;
            }
            break;
        case Mnemonic::kPseed:
            ensure_range(instruction.pseedid, 0, 3, "pseedid");
            idx0 = static_cast<std::uint32_t>(instruction.pseedid);
            break;
        case Mnemonic::kPsample:
            ensure_range(instruction.prd, 0, 15, "prd");
            idx0 = static_cast<std::uint32_t>(instruction.prd);
            break;
        case Mnemonic::kPmodld:
        case Mnemonic::kPmodsw:
            ensure_range(instruction.pmod, 0, 3, "pmod");
            idx0 = static_cast<std::uint32_t>(instruction.pmod);
            break;
        default:
            throw std::runtime_error("unexpected CFG mnemonic");
    }

    if (cfg > 0x7FF) {
        throw std::runtime_error("CFG is out of range");
    }

    std::uint32_t word = 0;
    word |= spec.class_code << 29;
    word |= spec.op_code << 26;
    word |= idx0 << 22;
    word |= idx1 << 18;
    word |= cfg << 7;
    word |= kCustom0Opcode;
    return word;
}

std::uint32_t encode_custom0_mem(const Instruction& instruction, const Custom0Spec& spec) {
    std::uint32_t preg = 0;
    if (instruction.mnemonic == Mnemonic::kSload) {
        ensure_range(instruction.prd, 0, 15, "prd");
        preg = static_cast<std::uint32_t>(instruction.prd);
    } else {
        ensure_range(instruction.prs1, 0, 15, "prs1");
        preg = static_cast<std::uint32_t>(instruction.prs1);
    }

    if (instruction.saddr > 0x7FFF) {
        throw std::runtime_error("saddr is out of range");
    }

    std::uint32_t word = 0;
    word |= spec.class_code << 29;
    word |= spec.op_code << 27;
    word |= static_cast<std::uint32_t>(instruction.interrupt_enable ? 1U : 0U) << 26;
    word |= preg << 22;
    word |= static_cast<std::uint32_t>(instruction.saddr) << 7;
    word |= kCustom0Opcode;
    return word;
}

std::uint32_t encode_custom1_dma(const Instruction& instruction) {
    ensure_range(instruction.rs1, 0, 31, "rs1");
    ensure_range(instruction.rs2, 0, 31, "rs2");
    ensure_range(instruction.rd, 0, 31, "rd");

    const std::uint32_t funct3 =
        instruction.mnemonic == Mnemonic::kDmaMemToHpu ? 0b000 : 0b001;
    // 文档只说明 IE 位位于 funct7 中，这里约定使用 funct7 的最低位 inst[25]。
    const std::uint32_t funct7 = instruction.interrupt_enable ? 0b0000001 : 0b0000000;

    std::uint32_t word = 0;
    word |= funct7 << 25;
    word |= static_cast<std::uint32_t>(instruction.rs2) << 20;
    word |= static_cast<std::uint32_t>(instruction.rs1) << 15;
    word |= funct3 << 12;
    word |= static_cast<std::uint32_t>(instruction.rd) << 7;
    word |= kCustom1Opcode;
    return word;
}

}  // namespace

std::uint32_t encode_instruction(const Instruction& instruction) {
    if (instruction.mnemonic == Mnemonic::kDmaMemToHpu ||
        instruction.mnemonic == Mnemonic::kDmaHpuToMem) {
        return encode_custom1_dma(instruction);
    }

    const Custom0Spec spec = lookup_custom0_spec(instruction.mnemonic);
    switch (spec.format) {
        case Format::kRRR:
            return encode_custom0_rrr(instruction, spec);
        case Format::kCFG:
            return encode_custom0_cfg(instruction, spec);
        case Format::kMEM:
            return encode_custom0_mem(instruction, spec);
        case Format::kDMA:
            break;
    }

    throw std::runtime_error("unsupported instruction format");
}

}  // namespace hpu
