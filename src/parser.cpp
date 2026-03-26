#include "hpu/parser.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace hpu {
namespace {

std::string trim(std::string value) {
    auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string strip_inline_comments(std::string line) {
    auto erase_from = [&](std::size_t pos) {
        if (pos != std::string::npos) {
            line.erase(pos);
        }
    };

    erase_from(line.find("//"));
    erase_from(line.find('#'));
    erase_from(line.find(';'));

    const auto block_begin = line.find("/*");
    if (block_begin != std::string::npos) {
        const auto block_end = line.find("*/", block_begin + 2);
        if (block_end != std::string::npos) {
            line.erase(block_begin, block_end - block_begin + 2);
        } else {
            line.erase(block_begin);
        }
    }

    return line;
}

std::string unescape_quoted(std::string_view quoted) {
    std::string out;
    out.reserve(quoted.size());
    bool escaping = false;
    for (char ch : quoted) {
        if (escaping) {
            switch (ch) {
                case 'n':
                case 'r':
                case 't':
                    break;
                case '\\':
                case '"':
                    out.push_back(ch);
                    break;
                default:
                    out.push_back(ch);
                    break;
            }
            escaping = false;
            continue;
        }

        if (ch == '\\') {
            escaping = true;
            continue;
        }

        out.push_back(ch);
    }
    return out;
}

std::string normalize_line(std::string_view raw_line) {
    std::string line = trim(strip_inline_comments(std::string(raw_line)));
    if (line.empty()) {
        return {};
    }

    const auto first_quote = line.find('"');
    const auto last_quote = line.rfind('"');
    if (first_quote != std::string::npos && last_quote != first_quote) {
        line = trim(unescape_quoted(line.substr(first_quote + 1, last_quote - first_quote - 1)));
    }

    line = trim(line);
    if (line == "__asm__ volatile(" || line == "asm volatile(" || line == ");" || line == "(" || line == ")") {
        return {};
    }

    return line;
}

std::vector<std::string> split_operands(const std::string& operand_text) {
    std::vector<std::string> operands;
    std::string current;
    std::stringstream ss(operand_text);
    while (std::getline(ss, current, ',')) {
        current = trim(current);
        if (!current.empty()) {
            operands.push_back(current);
        }
    }
    return operands;
}

int parse_base0_int(const std::string& token, const std::string& field_name) {
    std::size_t consumed = 0;
    int value = 0;
    try {
        value = std::stoi(token, &consumed, 0);
    } catch (const std::exception&) {
        throw std::runtime_error("invalid integer for " + field_name + ": " + token);
    }

    if (consumed != token.size()) {
        throw std::runtime_error("invalid integer for " + field_name + ": " + token);
    }
    return value;
}

int parse_prefixed_register(const std::string& token,
                            char prefix,
                            int max_value,
                            const std::string& field_name) {
    if (token.size() < 2 || token.front() != prefix) {
        throw std::runtime_error("invalid register for " + field_name + ": " + token);
    }

    const int value = parse_base0_int(token.substr(1), field_name);
    if (value < 0 || value > max_value) {
        throw std::runtime_error("register out of range for " + field_name + ": " + token);
    }
    return value;
}

void expect_operand_count(const std::vector<std::string>& operands,
                          std::size_t expected,
                          const std::string& mnemonic) {
    if (operands.size() != expected) {
        throw std::runtime_error("unexpected operand count for " + mnemonic);
    }
}

Instruction parse_custom0(const std::string& mnemonic, const std::vector<std::string>& operands) {
    Instruction instruction {};

    if (mnemonic == "padd") {
        expect_operand_count(operands, 3, mnemonic);
        instruction.mnemonic = Mnemonic::kPadd;
        instruction.prs1 = parse_prefixed_register(operands[0], 'p', 15, "prs1");
        instruction.prs2 = parse_prefixed_register(operands[1], 'p', 15, "prs2");
        instruction.prd = parse_prefixed_register(operands[2], 'p', 15, "prd");
        return instruction;
    }
    if (mnemonic == "psub") {
        expect_operand_count(operands, 3, mnemonic);
        instruction.mnemonic = Mnemonic::kPsub;
        instruction.prs1 = parse_prefixed_register(operands[0], 'p', 15, "prs1");
        instruction.prs2 = parse_prefixed_register(operands[1], 'p', 15, "prs2");
        instruction.prd = parse_prefixed_register(operands[2], 'p', 15, "prd");
        return instruction;
    }
    if (mnemonic == "pmul") {
        expect_operand_count(operands, 3, mnemonic);
        instruction.mnemonic = Mnemonic::kPmul;
        instruction.prs1 = parse_prefixed_register(operands[0], 'p', 15, "prs1");
        instruction.prs2 = parse_prefixed_register(operands[1], 'p', 15, "prs2");
        instruction.prd = parse_prefixed_register(operands[2], 'p', 15, "prd");
        return instruction;
    }
    if (mnemonic == "pmac") {
        expect_operand_count(operands, 3, mnemonic);
        instruction.mnemonic = Mnemonic::kPmac;
        instruction.prs1 = parse_prefixed_register(operands[0], 'p', 15, "prs1");
        instruction.prs2 = parse_prefixed_register(operands[1], 'p', 15, "prs2");
        instruction.prd = parse_prefixed_register(operands[2], 'p', 15, "prd");
        return instruction;
    }
    if (mnemonic == "pmov") {
        expect_operand_count(operands, 2, mnemonic);
        instruction.mnemonic = Mnemonic::kPmov;
        instruction.prs1 = parse_prefixed_register(operands[0], 'p', 15, "prs1");
        instruction.prd = parse_prefixed_register(operands[1], 'p', 15, "prd");
        return instruction;
    }
    if (mnemonic == "pbcast") {
        expect_operand_count(operands, 2, mnemonic);
        instruction.mnemonic = Mnemonic::kPbcast;
        instruction.pcst = parse_prefixed_register(operands[0], 'c', 3, "pcst");
        instruction.prd = parse_prefixed_register(operands[1], 'p', 15, "prd");
        return instruction;
    }
    if (mnemonic == "pntt") {
        expect_operand_count(operands, 3, mnemonic);
        instruction.mnemonic = Mnemonic::kPntt;
        instruction.prs1 = parse_prefixed_register(operands[0], 'p', 15, "prs1");
        instruction.prs2 = parse_prefixed_register(operands[1], 'p', 15, "prs2");
        instruction.prd = parse_prefixed_register(operands[2], 'p', 15, "prd");
        return instruction;
    }
    if (mnemonic == "pintt") {
        expect_operand_count(operands, 3, mnemonic);
        instruction.mnemonic = Mnemonic::kPintt;
        instruction.prs1 = parse_prefixed_register(operands[0], 'p', 15, "prs1");
        instruction.prs2 = parse_prefixed_register(operands[1], 'p', 15, "prs2");
        instruction.prd = parse_prefixed_register(operands[2], 'p', 15, "prd");
        return instruction;
    }
    if (mnemonic == "ptwld") {
        expect_operand_count(operands, 1, mnemonic);
        instruction.mnemonic = Mnemonic::kPtwld;
        instruction.ptw = parse_base0_int(operands[0], "ptw");
        return instruction;
    }
    if (mnemonic == "ptwid") {
        expect_operand_count(operands, 0, mnemonic);
        instruction.mnemonic = Mnemonic::kPtwid;
        return instruction;
    }
    if (mnemonic == "ptwi2") {
        expect_operand_count(operands, 0, mnemonic);
        instruction.mnemonic = Mnemonic::kPtwi2;
        return instruction;
    }
    if (mnemonic == "pshcfg") {
        expect_operand_count(operands, 1, mnemonic);
        instruction.mnemonic = Mnemonic::kPshcfg;
        instruction.pshf = parse_base0_int(operands[0], "pshf");
        return instruction;
    }
    if (mnemonic == "pshuf") {
        if (operands.size() != 2 && operands.size() != 3) {
            throw std::runtime_error("unexpected operand count for pshuf");
        }
        instruction.mnemonic = Mnemonic::kPshuf;
        instruction.prs1 = parse_prefixed_register(operands[0], 'p', 15, "prs1");
        instruction.prd = parse_prefixed_register(operands[1], 'p', 15, "prd");
        if (operands.size() == 3) {
            instruction.pshf = parse_base0_int(operands[2], "pshf");
        }
        return instruction;
    }
    if (mnemonic == "pshuf2") {
        if (operands.size() != 3 && operands.size() != 4) {
            throw std::runtime_error("unexpected operand count for pshuf2");
        }
        instruction.mnemonic = Mnemonic::kPshuf2;
        instruction.prs1 = parse_prefixed_register(operands[0], 'p', 15, "prs1");
        instruction.prs2 = parse_prefixed_register(operands[1], 'p', 15, "prs2");
        instruction.prd = parse_prefixed_register(operands[2], 'p', 15, "prd");
        if (operands.size() == 4) {
            instruction.pshf = parse_base0_int(operands[3], "pshf");
        }
        return instruction;
    }
    if (mnemonic == "pseed") {
        expect_operand_count(operands, 1, mnemonic);
        instruction.mnemonic = Mnemonic::kPseed;
        instruction.pseedid = parse_base0_int(operands[0], "pseedid");
        return instruction;
    }
    if (mnemonic == "psample") {
        expect_operand_count(operands, 1, mnemonic);
        instruction.mnemonic = Mnemonic::kPsample;
        instruction.prd = parse_prefixed_register(operands[0], 'p', 15, "prd");
        return instruction;
    }
    if (mnemonic == "pmodld") {
        expect_operand_count(operands, 1, mnemonic);
        instruction.mnemonic = Mnemonic::kPmodld;
        instruction.pmod = parse_base0_int(operands[0], "pmod");
        return instruction;
    }
    if (mnemonic == "pmodsw") {
        expect_operand_count(operands, 1, mnemonic);
        instruction.mnemonic = Mnemonic::kPmodsw;
        instruction.pmod = parse_base0_int(operands[0], "pmod");
        return instruction;
    }
    if (mnemonic == "sload") {
        expect_operand_count(operands, 2, mnemonic);
        instruction.mnemonic = Mnemonic::kSload;
        instruction.saddr = static_cast<std::uint16_t>(parse_base0_int(operands[0], "saddr"));
        instruction.prd = parse_prefixed_register(operands[1], 'p', 15, "prd");
        return instruction;
    }
    if (mnemonic == "sstore" || mnemonic == "sstore.irq" || mnemonic == "sstore.i") {
        expect_operand_count(operands, 2, mnemonic);
        instruction.mnemonic = Mnemonic::kSstore;
        instruction.interrupt_enable = (mnemonic != "sstore");
        instruction.prs1 = parse_prefixed_register(operands[0], 'p', 15, "prs1");
        instruction.saddr = static_cast<std::uint16_t>(parse_base0_int(operands[1], "saddr"));
        return instruction;
    }

    throw std::runtime_error("unsupported mnemonic: " + mnemonic);
}

Instruction parse_dma(const std::string& mnemonic, const std::vector<std::string>& operands) {
    expect_operand_count(operands, 3, mnemonic);

    Instruction instruction {};
    if (mnemonic == "dma.m2h" || mnemonic == "dma.mem2hpu" || mnemonic == "dma.m2h.irq") {
        instruction.mnemonic = Mnemonic::kDmaMemToHpu;
        instruction.interrupt_enable = (mnemonic == "dma.m2h.irq");
    } else if (mnemonic == "dma.h2m" || mnemonic == "dma.hpu2mem" || mnemonic == "dma.h2m.irq") {
        instruction.mnemonic = Mnemonic::kDmaHpuToMem;
        instruction.interrupt_enable = (mnemonic == "dma.h2m.irq");
    } else {
        throw std::runtime_error("unsupported DMA mnemonic: " + mnemonic);
    }

    instruction.rs1 = parse_prefixed_register(operands[0], 'x', 31, "rs1");
    instruction.rs2 = parse_prefixed_register(operands[1], 'x', 31, "rs2");
    instruction.rd = parse_prefixed_register(operands[2], 'x', 31, "rd");
    return instruction;
}

}  // namespace

Instruction parse_instruction_line(std::string_view line) {
    const std::string normalized = normalize_line(line);
    if (normalized.empty()) {
        throw std::runtime_error("empty instruction line");
    }

    const auto split = normalized.find_first_of(" \t");
    const std::string mnemonic = to_lower(normalized.substr(0, split));
    const std::string operand_text = split == std::string::npos ? std::string() : normalized.substr(split + 1);
    const std::vector<std::string> operands = split_operands(operand_text);

    if (mnemonic.rfind("dma.", 0) == 0) {
        return parse_dma(mnemonic, operands);
    }

    return parse_custom0(mnemonic, operands);
}

std::vector<Instruction> parse_source(std::string_view source) {
    std::vector<Instruction> instructions;
    std::stringstream ss{std::string(source)};
    std::string line;
    while (std::getline(ss, line)) {
        const std::string normalized = normalize_line(line);
        if (normalized.empty()) {
            continue;
        }
        instructions.push_back(parse_instruction_line(normalized));
    }
    return instructions;
}

}  // namespace hpu
