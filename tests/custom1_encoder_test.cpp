#include <gtest/gtest.h>

#include "hpu/assembler.hpp"

TEST(Custom1EncoderTest, EncodesMemoryToHpuDma) {
    const auto encoded = hpu::assemble_line("dma.m2h x1, x2, x3");
    EXPECT_EQ(encoded.word, 0x002081ABU);
}

TEST(Custom1EncoderTest, EncodesHpuToMemoryDmaWithInterrupt) {
    const auto encoded = hpu::assemble_line("dma.h2m.irq x10, x11, x12");
    EXPECT_EQ(encoded.word, 0x02B5162BU);
}
