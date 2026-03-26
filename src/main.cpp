#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include "hpu/assembler.hpp"

namespace {

std::string read_all(std::istream& input) {
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

}  // namespace

int main(int argc, char** argv) {
    try {
        std::string source;
        if (argc > 2) {
            std::cerr << "Usage: " << argv[0] << " [asm-file]\n";
            return 1;
        }

        if (argc == 2) {
            std::ifstream file(argv[1]);
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
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
