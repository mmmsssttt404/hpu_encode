#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "hpu/assembler.hpp"

namespace {

std::string read_all(std::istream& input) {
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

std::string format_word_bits(std::uint32_t word) {
    std::string bits;
    bits.reserve(32);
    for (int bit = 31; bit >= 0; --bit) {
        bits.push_back(((word >> bit) & 1U) ? '1' : '0');
    }
    return bits;
}

std::filesystem::path instruction_output_path(const std::filesystem::path& input_path) {
    const auto stem = input_path.stem().string();
    const auto filename = stem.empty() ? input_path.filename().string() : stem;
    return input_path.parent_path() / (filename + ".inst32");
}

bool write_instruction_file(const std::filesystem::path& input_path,
                            const std::vector<hpu::EncodedInstruction>& encoded) {
    const auto output_path = instruction_output_path(input_path);
    std::ofstream output(output_path);
    if (!output) {
        return false;
    }

    for (const auto& item : encoded) {
        output << format_word_bits(item.word) << '\n';
    }
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        std::string source;
        std::filesystem::path input_path;
        if (argc > 2) {
            std::cerr << "Usage: " << argv[0] << " [asm-file]\n";
            return 1;
        }

        if (argc == 2) {
            input_path = argv[1];
            std::ifstream file(input_path);
            if (!file) {
                std::cerr << "Failed to open file: " << argv[1] << '\n';
                return 1;
            }
            source = read_all(file);
        } else {
            source = read_all(std::cin);
        }

        const auto encoded = hpu::assemble_source(source);
        for (const auto& item : encoded) {
            std::cout << hpu::format_word_hex(item.word) << "  " << item.normalized_asm << '\n';
        }

        if (argc == 2 && !write_instruction_file(input_path, encoded)) {
            std::cerr << "Failed to write instruction file for: " << input_path << '\n';
            return 1;
        }
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
