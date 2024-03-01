#pragma once

#include <cstdint>
#include <string_view>

using bytes_view = std::basic_string_view<uint8_t>;

enum class Result {
  zero,
  one,
  invalid_input_length,
  invalid_g1,
  invalid_g2,

  MAX,
};

inline constexpr size_t STRIDE_SIZE = 192;
inline constexpr size_t FE_SIZE = 32;

Result libff_pairing_verify(bytes_view input) noexcept;
