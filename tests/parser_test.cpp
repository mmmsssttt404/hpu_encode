#include <gtest/gtest.h>

#include "hpu/parser.hpp"

TEST(ParserTest, ParsesHexAndDecimalSramAddresses) {
    const auto decimal = hpu::parse_instruction_line("sload 256, p0");
    EXPECT_EQ(decimal.saddr, 256);
    EXPECT_EQ(decimal.prd, 0);

    const auto hex = hpu::parse_instruction_line("sstore.irq p2, 0x300");
    EXPECT_EQ(hex.saddr, 0x300);
    EXPECT_EQ(hex.prs1, 2);
    EXPECT_TRUE(hex.interrupt_enable);
}

TEST(ParserTest, ParsesQuotedInlineAsmLine) {
    const auto inst = hpu::parse_instruction_line("\"pmul p0, p1, p2 \\n\\t\"");
    EXPECT_EQ(inst.prs1, 0);
    EXPECT_EQ(inst.prs2, 1);
    EXPECT_EQ(inst.prd, 2);
}

TEST(ParserTest, ParsesCustom1Aliases) {
    const auto inst = hpu::parse_instruction_line("dma.h2m.irq x10, x11, x12");
    EXPECT_TRUE(inst.interrupt_enable);
    EXPECT_EQ(inst.rs1, 10);
    EXPECT_EQ(inst.rs2, 11);
    EXPECT_EQ(inst.rd, 12);
}
