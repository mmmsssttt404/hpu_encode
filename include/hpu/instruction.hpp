#pragma once

#include <cstdint>
#include <string>

namespace hpu {

enum class Format {
    kRRR,
    kCFG,
    kMEM,
    kDMA,
};

enum class Mnemonic {
    kPadd,
    kPsub,
    kPmul,
    kPmac,
    kPmov,
    kPbcast,
    kPntt,
    kPintt,
    kPtwld,
    kPtwid,
    kPtwi2,
    kPshcfg,
    kPshuf,
    kPshuf2,
    kPseed,
    kPsample,
    kPmodld,
    kPmodsw,
    kSload,
    kSstore,
    kDmaMemToHpu,
    kDmaHpuToMem,
};

struct Instruction {
    Mnemonic mnemonic {};
    bool interrupt_enable = false;

    int prd = -1;
    int prs1 = -1;
    int prs2 = -1;
    int pcst = -1;

    int ptw = -1;
    int pshf = -1;
    int pseedid = -1;
    int pmod = -1;

    std::uint16_t cfg = 0;
    std::uint8_t flag = 0;
    std::uint16_t saddr = 0;

    int rs1 = -1;
    int rs2 = -1;
    int rd = -1;
};

struct EncodedInstruction {
    Instruction instruction;
    std::uint32_t word = 0;
    std::string normalized_asm;
};

std::string to_string(Mnemonic mnemonic);
std::string to_string(const Instruction& instruction);
Format instruction_format(Mnemonic mnemonic);

}  // namespace hpu
