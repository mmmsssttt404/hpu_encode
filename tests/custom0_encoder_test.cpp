#include <gtest/gtest.h>

#include "hpu/assembler.hpp"

TEST(Custom0EncoderTest, EncodesRrrInstruction) {
    const auto encoded = hpu::assemble_line("pmul p0, p1, p2");
    EXPECT_EQ(encoded.word, 0x0880400BU);
}

TEST(Custom0EncoderTest, EncodesCfgInstruction) {
    const auto encoded = hpu::assemble_line("pbcast c3, p4");
    EXPECT_EQ(encoded.word, 0x150C000BU);
}

TEST(Custom0EncoderTest, EncodesMemInstructionWithInterrupt) {
    const auto encoded = hpu::assemble_line("sstore.irq p2, 0x300");
    EXPECT_EQ(encoded.word, 0xCC81800BU);
}

TEST(Custom0EncoderTest, EncodesExtendedCfgIndices) {
    const auto tw = hpu::assemble_line("ptwld 5");
    EXPECT_EQ(tw.word, 0x4140000BU);

    const auto sh = hpu::assemble_line("pshcfg 12");
    EXPECT_EQ(sh.word, 0x6300000BU);
}

TEST(Custom0EncoderTest, EncodesLargeShuffleTemplateIds) {
    const auto sh18 = hpu::assemble_line("pshcfg 18");
    EXPECT_EQ(sh18.word, 0x6080008BU);

    const auto sh26 = hpu::assemble_line("pshcfg 26");
    EXPECT_EQ(sh26.word, 0x6280008BU);
}
