#include <gtest/gtest.h>

#include "hpu/assembler.hpp"

TEST(AssemblerTest, AssemblesMultiLineInlineAsmSnippet) {
    const std::string source = R"(
__asm__ volatile(
    "pmodsw 1 \n\t"
    "sload 0x100, p0 \n\t"
    "pmul p0, p1, p2 \n\t"
    "dma.h2m x5, x6, x7 \n\t"
);
)";

    const auto encoded = hpu::assemble_source(source);
    ASSERT_EQ(encoded.size(), 4U);
    EXPECT_EQ(encoded[0].normalized_asm, "pmodsw 1");
    EXPECT_EQ(encoded[1].word, 0xC000800BU);
    EXPECT_EQ(encoded[2].word, 0x0880400BU);
    EXPECT_EQ(encoded[3].word, 0x006293ABU);
}
