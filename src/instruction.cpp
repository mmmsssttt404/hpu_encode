#include "hpu/instruction.hpp"

#include <sstream>
#include <stdexcept>

namespace hpu {
namespace {

std::string format_preg(int value) {
    return "p" + std::to_string(value);
}

std::string format_creg(int value) {
    return "c" + std::to_string(value);
}

std::string format_xreg(int value) {
    return "x" + std::to_string(value);
}

}  // namespace

std::string to_string(Mnemonic mnemonic) {
    switch (mnemonic) {
        case Mnemonic::kPadd: return "padd";
        case Mnemonic::kPsub: return "psub";
        case Mnemonic::kPmul: return "pmul";
        case Mnemonic::kPmac: return "pmac";
        case Mnemonic::kPmov: return "pmov";
        case Mnemonic::kPbcast: return "pbcast";
        case Mnemonic::kPntt: return "pntt";
        case Mnemonic::kPintt: return "pintt";
        case Mnemonic::kPtwld: return "ptwld";
        case Mnemonic::kPtwid: return "ptwid";
        case Mnemonic::kPtwi2: return "ptwi2";
        case Mnemonic::kPshcfg: return "pshcfg";
        case Mnemonic::kPshuf: return "pshuf";
        case Mnemonic::kPshuf2: return "pshuf2";
        case Mnemonic::kPseed: return "pseed";
        case Mnemonic::kPsample: return "psample";
        case Mnemonic::kPmodld: return "pmodld";
        case Mnemonic::kPmodsw: return "pmodsw";
        case Mnemonic::kSload: return "sload";
        case Mnemonic::kSstore: return "sstore";
        case Mnemonic::kDmaMemToHpu: return "dma.m2h";
        case Mnemonic::kDmaHpuToMem: return "dma.h2m";
    }

    throw std::runtime_error("unknown mnemonic");
}

Format instruction_format(Mnemonic mnemonic) {
    switch (mnemonic) {
        case Mnemonic::kPadd:
        case Mnemonic::kPsub:
        case Mnemonic::kPmul:
        case Mnemonic::kPmac:
        case Mnemonic::kPmov:
        case Mnemonic::kPntt:
        case Mnemonic::kPintt:
        case Mnemonic::kPshuf2:
            return Format::kRRR;

        case Mnemonic::kPbcast:
        case Mnemonic::kPtwld:
        case Mnemonic::kPtwid:
        case Mnemonic::kPtwi2:
        case Mnemonic::kPshcfg:
        case Mnemonic::kPshuf:
        case Mnemonic::kPseed:
        case Mnemonic::kPsample:
        case Mnemonic::kPmodld:
        case Mnemonic::kPmodsw:
            return Format::kCFG;

        case Mnemonic::kSload:
        case Mnemonic::kSstore:
            return Format::kMEM;

        case Mnemonic::kDmaMemToHpu:
        case Mnemonic::kDmaHpuToMem:
            return Format::kDMA;
    }

    throw std::runtime_error("unknown format for mnemonic");
}

std::string to_string(const Instruction& instruction) {
    std::ostringstream oss;
    oss << to_string(instruction.mnemonic);
    if (instruction.interrupt_enable &&
        (instruction.mnemonic == Mnemonic::kSstore ||
         instruction.mnemonic == Mnemonic::kDmaMemToHpu ||
         instruction.mnemonic == Mnemonic::kDmaHpuToMem)) {
        oss << ".irq";
    }

    switch (instruction_format(instruction.mnemonic)) {
        case Format::kRRR:
            if (instruction.mnemonic == Mnemonic::kPmov) {
                oss << ' ' << format_preg(instruction.prs1)
                    << ", " << format_preg(instruction.prd);
            } else {
                oss << ' ' << format_preg(instruction.prs1)
                    << ", " << format_preg(instruction.prs2)
                    << ", " << format_preg(instruction.prd);
                if (instruction.mnemonic == Mnemonic::kPshuf2 && instruction.pshf >= 0) {
                    oss << ", " << instruction.pshf;
                }
            }
            break;

        case Format::kCFG:
            switch (instruction.mnemonic) {
                case Mnemonic::kPbcast:
                    oss << ' ' << format_creg(instruction.pcst)
                        << ", " << format_preg(instruction.prd);
                    break;
                case Mnemonic::kPtwld:
                    oss << ' ' << instruction.ptw;
                    break;
                case Mnemonic::kPtwid:
                case Mnemonic::kPtwi2:
                    break;
                case Mnemonic::kPshcfg:
                    oss << ' ' << instruction.pshf;
                    break;
                case Mnemonic::kPshuf:
                    oss << ' ' << format_preg(instruction.prs1)
                        << ", " << format_preg(instruction.prd);
                    if (instruction.pshf >= 0) {
                        oss << ", " << instruction.pshf;
                    }
                    break;
                case Mnemonic::kPseed:
                    oss << ' ' << instruction.pseedid;
                    break;
                case Mnemonic::kPsample:
                    oss << ' ' << format_preg(instruction.prd);
                    break;
                case Mnemonic::kPmodld:
                case Mnemonic::kPmodsw:
                    oss << ' ' << instruction.pmod;
                    break;
                default:
                    throw std::runtime_error("unexpected CFG mnemonic");
            }
            break;

        case Format::kMEM:
            if (instruction.mnemonic == Mnemonic::kSload) {
                oss << ' ' << instruction.saddr << ", " << format_preg(instruction.prd);
            } else {
                oss << ' ' << format_preg(instruction.prs1) << ", " << instruction.saddr;
            }
            break;

        case Format::kDMA:
            oss << ' ' << format_xreg(instruction.rs1)
                << ", " << format_xreg(instruction.rs2)
                << ", " << format_xreg(instruction.rd);
            break;
    }

    return oss.str();
}

}  // namespace hpu
