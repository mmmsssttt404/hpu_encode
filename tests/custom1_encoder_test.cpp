#include <gtest/gtest.h>

#include "hpu/encoder.hpp"

TEST(Custom1EncoderTest, EncodesDloadAndDstore) {
    hpu::Instruction load {};
    load.mnemonic = hpu::Mnemonic::kDload;
    load.rs1 = 10;
    load.rs2 = 11;
    load.obj_id = 2;
    load.type = 2;
    EXPECT_EQ(hpu::encode_instruction(load), 0x00B5242BU);

    hpu::Instruction store {};
    store.mnemonic = hpu::Mnemonic::kDstore;
    store.rs1 = 14;
    store.rs2 = 15;
    store.obj_id = 1;
    store.type = 1;
    EXPECT_EQ(hpu::encode_instruction(store), 0x00F7522BU);
}
