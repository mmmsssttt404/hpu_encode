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
        case hpu::Format::kRRR:
            oss << bits(word, 31, 29) << '|'
                << bits(word, 28, 26) << '|'
                << bits(word, 25, 22) << '|'
                << bits(word, 21, 18) << '|'
                << bits(word, 17, 14) << '|'
                << bits(word, 13, 7) << '|'
                << bits(word, 6, 0)
                << "   (CLASS|OP|prd|prs1|prs2|FLAG|opcode)";
            break;
        case hpu::Format::kCFG:
            oss << bits(word, 31, 29) << '|'
                << bits(word, 28, 26) << '|'
                << bits(word, 25, 22) << '|'
                << bits(word, 21, 18) << '|'
                << bits(word, 17, 7) << '|'
                << bits(word, 6, 0)
                << "   (CLASS|OP|IDX0|IDX1|CFG|opcode)";
            break;
        case hpu::Format::kMEM:
            oss << bits(word, 31, 29) << '|'
                << bits(word, 28, 27) << '|'
                << bits(word, 26, 26) << '|'
                << bits(word, 25, 22) << '|'
                << bits(word, 21, 7) << '|'
                << bits(word, 6, 0)
                << "   (CLASS|OP|IE|preg|saddr|opcode)";
            break;
        case hpu::Format::kDMA:
            oss << bits(word, 31, 25) << '|'
                << bits(word, 24, 20) << '|'
                << bits(word, 19, 15) << '|'
                << bits(word, 14, 12) << '|'
                << bits(word, 11, 7) << '|'
                << bits(word, 6, 0)
                << "   (funct7|rs2|rs1|funct3|rd|opcode)";
            break;
    }
    return oss.str();
}

std::string format_fields(const hpu::EncodedInstruction& encoded) {
    const auto format = hpu::instruction_format(encoded.instruction.mnemonic);
    const std::uint32_t word = encoded.word;

    std::ostringstream oss;
    switch (format) {
        case hpu::Format::kRRR:
            oss << "CLASS=" << field(word, 31, 29) << '(' << bits(word, 31, 29) << ") "
                << "OP=" << field(word, 28, 26) << '(' << bits(word, 28, 26) << ") "
                << "prd=" << field(word, 25, 22) << '(' << bits(word, 25, 22) << ") "
                << "prs1=" << field(word, 21, 18) << '(' << bits(word, 21, 18) << ") "
                << "prs2=" << field(word, 17, 14) << '(' << bits(word, 17, 14) << ") "
                << "FLAG=" << field(word, 13, 7) << '(' << bits(word, 13, 7) << ") "
                << "opcode=" << field(word, 6, 0) << '(' << bits(word, 6, 0) << ')';
            break;
        case hpu::Format::kCFG:
            oss << "CLASS=" << field(word, 31, 29) << '(' << bits(word, 31, 29) << ") "
                << "OP=" << field(word, 28, 26) << '(' << bits(word, 28, 26) << ") "
                << "IDX0=" << field(word, 25, 22) << '(' << bits(word, 25, 22) << ") "
                << "IDX1=" << field(word, 21, 18) << '(' << bits(word, 21, 18) << ") "
                << "CFG=" << field(word, 17, 7) << '(' << bits(word, 17, 7) << ") "
                << "opcode=" << field(word, 6, 0) << '(' << bits(word, 6, 0) << ')';
            break;
        case hpu::Format::kMEM:
            oss << "CLASS=" << field(word, 31, 29) << '(' << bits(word, 31, 29) << ") "
                << "OP=" << field(word, 28, 27) << '(' << bits(word, 28, 27) << ") "
                << "IE=" << field(word, 26, 26) << '(' << bits(word, 26, 26) << ") "
                << "preg=" << field(word, 25, 22) << '(' << bits(word, 25, 22) << ") "
                << "saddr=" << field(word, 21, 7) << '(' << bits(word, 21, 7) << ") "
                << "opcode=" << field(word, 6, 0) << '(' << bits(word, 6, 0) << ')';
            break;
        case hpu::Format::kDMA:
            oss << "funct7=" << field(word, 31, 25) << '(' << bits(word, 31, 25) << ") "
                << "rs2=" << field(word, 24, 20) << '(' << bits(word, 24, 20) << ") "
                << "rs1=" << field(word, 19, 15) << '(' << bits(word, 19, 15) << ") "
                << "funct3=" << field(word, 14, 12) << '(' << bits(word, 14, 12) << ") "
                << "rd=" << field(word, 11, 7) << '(' << bits(word, 11, 7) << ") "
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
