#include <gtest/gtest.h>

#include "hpu/parser.hpp"

TEST(ParserTest, ParsesStageInstructionWithFiveOperands) {
    const auto instruction = hpu::parse_instruction_line("pntt p1, p0, 5, 0, 0");
    EXPECT_EQ(instruction.mnemonic, hpu::Mnemonic::kPntt);
    EXPECT_EQ(instruction.pdst, 1);
    EXPECT_EQ(instruction.psrc1, 0);
    EXPECT_EQ(instruction.idx0, 5);
    EXPECT_EQ(instruction.idx1, 0);
    EXPECT_EQ(instruction.mode, 0);
}

TEST(ParserTest, ParsesCfgAndSyncInstructions) {
    const auto cfg = hpu::parse_instruction_line("pmodld p2, 0, 0");
    EXPECT_EQ(cfg.mnemonic, hpu::Mnemonic::kPmodld);
    EXPECT_EQ(cfg.idx0, 2);
    EXPECT_EQ(cfg.idx1, 0);
    EXPECT_EQ(cfg.cfg, 0);

    const auto sync = hpu::parse_instruction_line("psync 7, 3");
    EXPECT_EQ(sync.mnemonic, hpu::Mnemonic::kPsync);
    EXPECT_EQ(sync.tag, 7);
    EXPECT_EQ(sync.mode, 3);
}

TEST(ParserTest, ParsesInlineAsmAndCustom1AliasesFromSource) {
    const std::string source = R"(
__asm__ volatile(
    "pmodld p2, 0, 0 \n\t"
    "dload x10, x11, p3, 2 \n\t"
    "psync 0, 0 \n\t"
);
)";

    const auto instructions = hpu::parse_source(source);
    ASSERT_EQ(instructions.size(), 3U);
    EXPECT_EQ(instructions[0].mnemonic, hpu::Mnemonic::kPmodld);
    EXPECT_EQ(instructions[1].mnemonic, hpu::Mnemonic::kDload);
    EXPECT_EQ(instructions[1].obj_id, 3);
    EXPECT_EQ(instructions[1].type, 2);
    EXPECT_EQ(instructions[2].mnemonic, hpu::Mnemonic::kPsync);
}
