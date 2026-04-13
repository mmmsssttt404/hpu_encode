#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

#include "hpu/assembler.hpp"

namespace {

std::filesystem::path instruction_output_path(const std::filesystem::path& input_path) {
    const auto stem = input_path.stem().string();
    const auto filename = stem.empty() ? input_path.filename().string() : stem;
    return input_path.parent_path() / (filename + ".inst32");
}

std::string read_file(const std::filesystem::path& path) {
    std::ifstream input(path);
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

}  // namespace

TEST(CliOutputTest, WritesPlain32BitInstructionFileForAsmInput) {
    const std::string source =
        "pmodld p2, 0, 0\n"
        "pmul p0, p1, p2\n"
        "psync 0, 0\n";

    const auto temp_dir = std::filesystem::temp_directory_path();
    const auto asm_path = temp_dir / "hpu_cli_output_test.asm";
    const auto inst_path = instruction_output_path(asm_path);

    {
        std::ofstream asm_file(asm_path);
        ASSERT_TRUE(asm_file.is_open());
        asm_file << source;
    }

    std::filesystem::remove(inst_path);

    const std::string command = "./hpu_encode_cli " + asm_path.string() + " > /dev/null";
    ASSERT_EQ(std::system(command.c_str()), 0);
    ASSERT_TRUE(std::filesystem::exists(inst_path));

    std::string expected;
    for (const auto& item : hpu::assemble_source(source)) {
        expected += format_word_bits(item.word);
        expected.push_back('\n');
    }
    EXPECT_EQ(read_file(inst_path), expected);

    std::filesystem::remove(asm_path);
    std::filesystem::remove(inst_path);
}
