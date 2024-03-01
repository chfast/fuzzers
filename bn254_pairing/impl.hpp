#pragma once

#include <cstdint>
#include <string_view>

using bytes_view = std::basic_string_view<uint8_t>;

enum class Result {
  zero = 0,
  one = 1,
  invalid_input_length = -1,
  invalid_g1 = -2,
  invalid_g2 = -3,
};

inline constexpr size_t INPUT_STRIDE = 192;

Result libff_pairing_verify(bytes_view input) noexcept;
