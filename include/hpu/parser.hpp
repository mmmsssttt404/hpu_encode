#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "hpu/instruction.hpp"

namespace hpu {

Instruction parse_instruction_line(std::string_view line);
std::vector<Instruction> parse_source(std::string_view source);

}  // namespace hpu
