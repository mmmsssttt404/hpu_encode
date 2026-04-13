#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "hpu/assembler.hpp"
#include "hpu/instruction.hpp"

namespace {

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool should_skip_line(const std::string& line) {
    if (line.empty()) {
        return true;
    }
    return line.rfind('#', 0) == 0 || line.rfind("//", 0) == 0 || line.rfind("/*", 0) == 0;
}

std::uint32_t field(std::uint32_t word, int hi, int lo) {
    const std::uint32_t width = static_cast<std::uint32_t>(hi - lo + 1);
    const std::uint32_t mask = width == 32 ? 0xFFFFFFFFu : ((1u << width) - 1u);
    return (word >> lo) & mask;
}

std::string bits(std::uint32_t word, int hi, int lo) {
    std::string out;
    for (int bit = hi; bit >= lo; --bit) {
        out.push_back(((word >> bit) & 1U) ? '1' : '0');
    }
    return out;
}

std::string format_segmented_bits(const hpu::EncodedInstruction& encoded) {
    const auto format = hpu::instruction_format(encoded.instruction.mnemonic);
    const std::uint32_t word = encoded.word;

    std::ostringstream oss;
    switch (format) {
        case hpu::Format::kAR3:
            oss << bits(word, 31, 28) << '|'
                << bits(word, 27, 25) << '|'
                << bits(word, 24, 22) << '|'
                << bits(word, 21, 14) << '|'
                << bits(word, 13, 10) << '|'
                << bits(word, 9, 7) << '|'
                << bits(word, 6, 0)
                << "   (OPC|pdst|psrc1|OP2|MODE|FLAG|opcode)";
            break;
        case hpu::Format::kSTG:
            oss << bits(word, 31, 28) << '|'
                << bits(word, 27, 25) << '|'
                << bits(word, 24, 22) << '|'
                << bits(word, 21, 18) << '|'
                << bits(word, 17, 14) << '|'
                << bits(word, 13, 10) << '|'
                << bits(word, 9, 7) << '|'
                << bits(word, 6, 0)
                << "   (OPC|pdst|psrc|IDX0|IDX1|MODE|FLAG|opcode)";
            break;
        case hpu::Format::kCFG:
            if (encoded.instruction.mnemonic == hpu::Mnemonic::kPseed) {
                oss << bits(word, 31, 28) << '|'
                    << bits(word, 27, 7) << '|'
                    << bits(word, 6, 0)
                    << "   (OPC|IMM21|opcode)";
            } else {
                oss << bits(word, 31, 28) << '|'
                    << bits(word, 27, 25) << '|'
                    << bits(word, 24, 22) << '|'
                    << bits(word, 21, 7) << '|'
                    << bits(word, 6, 0)
                    << "   (OPC|IDX0|IDX1|CFG|opcode)";
            }
            break;
        case hpu::Format::kSYNC:
            oss << bits(word, 31, 28) << '|'
                << bits(word, 27, 23) << '|'
                << bits(word, 22, 20) << '|'
                << bits(word, 19, 7) << '|'
                << bits(word, 6, 0)
                << "   (OPC|TAG|MODE|RSV|opcode)";
            break;
        case hpu::Format::kDMA:
            oss << bits(word, 31, 25) << '|'
                << bits(word, 24, 20) << '|'
                << bits(word, 19, 15) << '|'
                << bits(word, 14, 14) << '|'
                << bits(word, 13, 12) << '|'
                << bits(word, 11, 9) << '|'
                << bits(word, 8, 7) << '|'
                << bits(word, 6, 0)
                << "   (funct7|rs2|rs1|DIR|TYPE|obj_id|RSV2|opcode)";
            break;
    }
    return oss.str();
}

std::string format_fields(const hpu::EncodedInstruction& encoded) {
    const auto format = hpu::instruction_format(encoded.instruction.mnemonic);
    const std::uint32_t word = encoded.word;

    std::ostringstream oss;
    switch (format) {
        case hpu::Format::kAR3:
            oss << "OPC=" << field(word, 31, 28) << '(' << bits(word, 31, 28) << ") "
                << "pdst=" << field(word, 27, 25) << '(' << bits(word, 27, 25) << ") "
                << "psrc1=" << field(word, 24, 22) << '(' << bits(word, 24, 22) << ") "
                << "OP2=" << field(word, 21, 14) << '(' << bits(word, 21, 14) << ") "
                << "MODE=" << field(word, 13, 10) << '(' << bits(word, 13, 10) << ") "
                << "FLAG=" << field(word, 9, 7) << '(' << bits(word, 9, 7) << ") "
                << "opcode=" << field(word, 6, 0) << '(' << bits(word, 6, 0) << ')';
            break;
        case hpu::Format::kSTG:
            oss << "OPC=" << field(word, 31, 28) << '(' << bits(word, 31, 28) << ") "
                << "pdst=" << field(word, 27, 25) << '(' << bits(word, 27, 25) << ") "
                << "psrc=" << field(word, 24, 22) << '(' << bits(word, 24, 22) << ") "
                << "IDX0=" << field(word, 21, 18) << '(' << bits(word, 21, 18) << ") "
                << "IDX1=" << field(word, 17, 14) << '(' << bits(word, 17, 14) << ") "
                << "MODE=" << field(word, 13, 10) << '(' << bits(word, 13, 10) << ") "
                << "FLAG=" << field(word, 9, 7) << '(' << bits(word, 9, 7) << ") "
                << "opcode=" << field(word, 6, 0) << '(' << bits(word, 6, 0) << ')';
            break;
        case hpu::Format::kCFG:
            if (encoded.instruction.mnemonic == hpu::Mnemonic::kPseed) {
                oss << "OPC=" << field(word, 31, 28) << '(' << bits(word, 31, 28) << ") "
                    << "IMM21=" << field(word, 27, 7) << '(' << bits(word, 27, 7) << ") "
                    << "opcode=" << field(word, 6, 0) << '(' << bits(word, 6, 0) << ')';
            } else {
                oss << "OPC=" << field(word, 31, 28) << '(' << bits(word, 31, 28) << ") "
                    << "IDX0=" << field(word, 27, 25) << '(' << bits(word, 27, 25) << ") "
                    << "IDX1=" << field(word, 24, 22) << '(' << bits(word, 24, 22) << ") "
                    << "CFG=" << field(word, 21, 7) << '(' << bits(word, 21, 7) << ") "
                    << "opcode=" << field(word, 6, 0) << '(' << bits(word, 6, 0) << ')';
            }
            break;
        case hpu::Format::kSYNC:
            oss << "OPC=" << field(word, 31, 28) << '(' << bits(word, 31, 28) << ") "
                << "TAG=" << field(word, 27, 23) << '(' << bits(word, 27, 23) << ") "
                << "MODE=" << field(word, 22, 20) << '(' << bits(word, 22, 20) << ") "
                << "RSV=" << field(word, 19, 7) << '(' << bits(word, 19, 7) << ") "
                << "opcode=" << field(word, 6, 0) << '(' << bits(word, 6, 0) << ')';
            break;
        case hpu::Format::kDMA:
            oss << "funct7=" << field(word, 31, 25) << '(' << bits(word, 31, 25) << ") "
                << "rs2=" << field(word, 24, 20) << '(' << bits(word, 24, 20) << ") "
                << "rs1=" << field(word, 19, 15) << '(' << bits(word, 19, 15) << ") "
                << "DIR=" << field(word, 14, 14) << '(' << bits(word, 14, 14) << ") "
                << "TYPE=" << field(word, 13, 12) << '(' << bits(word, 13, 12) << ") "
                << "obj_id=" << field(word, 11, 9) << '(' << bits(word, 11, 9) << ") "
                << "RSV2=" << field(word, 8, 7) << '(' << bits(word, 8, 7) << ") "
                << "opcode=" << field(word, 6, 0) << '(' << bits(word, 6, 0) << ')';
            break;
    }
    return oss.str();
}

std::string format_hex(std::uint32_t word) {
    std::ostringstream oss;
    oss << "0x" << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << word;
    return oss.str();
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <asm-file>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Failed to open file: " << argv[1] << '\n';
        return 1;
    }

    bool has_error = false;
    std::string raw_line;
    int line_no = 0;
    while (std::getline(file, raw_line)) {
        ++line_no;
        const std::string line = trim(raw_line);
        if (should_skip_line(line)) {
            continue;
        }

        try {
            const auto encoded = hpu::assemble_line(line);
            std::cout << '[' << line_no << "] " << encoded.normalized_asm << '\n';
            std::cout << "     bits   : " << format_segmented_bits(encoded) << '\n';
            std::cout << "     fields : " << format_fields(encoded) << '\n';
            std::cout << "     hex    : " << format_hex(encoded.word) << "\n\n";
        } catch (const std::exception& ex) {
            has_error = true;
            std::cout << '[' << line_no << "] " << line << '\n';
            std::cout << "     ERROR: " << ex.what() << "\n\n";
        }
    }

    return has_error ? 2 : 0;
}
