#include <gtest/gtest.h>

#include "hpu/encoder.hpp"

TEST(Custom0EncoderTest, EncodesAr3ObjectAndImmediateModes) {
    hpu::Instruction obj {};
    obj.mnemonic = hpu::Mnemonic::kPmul;
    obj.pdst = 0;
    obj.psrc1 = 1;
    obj.psrc2 = 2;
    EXPECT_EQ(hpu::encode_instruction(obj), 0x2040800BU);

    hpu::Instruction imm {};
    imm.mnemonic = hpu::Mnemonic::kPmuli;
    imm.pdst = 0;
    imm.psrc1 = 1;
    imm.imm8 = 7;
    EXPECT_EQ(hpu::encode_instruction(imm), 0x2041C40BU);
}

TEST(Custom0EncoderTest, EncodesStageCfgAndSyncInstructions) {
    hpu::Instruction stg {};
    stg.mnemonic = hpu::Mnemonic::kPntt;
    stg.pdst = 1;
    stg.psrc1 = 0;
    stg.idx0 = 5;
    stg.idx1 = 0;
    stg.mode = 0;
    EXPECT_EQ(hpu::encode_instruction(stg), 0x4214000BU);

    hpu::Instruction cfg {};
    cfg.mnemonic = hpu::Mnemonic::kPmodld;
    cfg.idx0 = 2;
    cfg.idx1 = 0;
    cfg.cfg = 0;
    EXPECT_EQ(hpu::encode_instruction(cfg), 0xA400000BU);

    hpu::Instruction sync {};
    sync.mnemonic = hpu::Mnemonic::kPsync;
    sync.tag = 3;
    sync.mode = 2;
    EXPECT_EQ(hpu::encode_instruction(sync), 0xB1A0000BU);
}

TEST(Custom0EncoderTest, EncodesSeedImmediate) {
    hpu::Instruction seed {};
    seed.mnemonic = hpu::Mnemonic::kPseed;
    seed.imm21 = 0x15555;
    EXPECT_EQ(hpu::encode_instruction(seed), 0x80AAAA8BU);
}
