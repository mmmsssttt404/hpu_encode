#include <gtest/gtest.h>

#include "hpu/assembler.hpp"

TEST(AssemblerTest, AssemblesMultiLineInlineAsmSnippet) {
    const std::string source = R"(
__asm__ volatile(
    "pmodld p2, 0, 0 \n\t"
    "pntt p1, p0, 2, 0, 0 \n\t"
    "dload x10, x11, p3, 2 \n\t"
    "psync 0, 0 \n\t"
);
)";

    const auto encoded = hpu::assemble_source(source);
    ASSERT_EQ(encoded.size(), 4U);
    EXPECT_EQ(encoded[0].normalized_asm, "pmodld p2, 0, 0");
    EXPECT_EQ(encoded[0].word, 0xA400000BU);
    EXPECT_EQ(encoded[1].normalized_asm, "pntt p1, p0, 2, 0, 0");
    EXPECT_EQ(encoded[1].word, 0x4208000BU);
    EXPECT_EQ(encoded[2].normalized_asm, "dload x10, x11, p3, 2");
    EXPECT_EQ(encoded[2].word, 0x00B5262BU);
    EXPECT_EQ(encoded[3].normalized_asm, "psync 0, 0");
    EXPECT_EQ(encoded[3].word, 0xB000000BU);
}
