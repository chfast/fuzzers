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
  invalid_g2_subgroup,

  MAX,
};

inline constexpr size_t STRIDE_SIZE = 192;
inline constexpr size_t FE_SIZE = 32;

Result libff_pairing_verify(bytes_view input) noexcept;

void libff_generate_abc(uint8_t out[2 * STRIDE_SIZE],
                        const uint8_t scalars_data[2 * FE_SIZE]);

bool libff_generate_abcd(uint8_t out[2 * STRIDE_SIZE],
                         const uint8_t scalars_data[3 * FE_SIZE]);

bool libff_generate_wrong_g2(uint8_t data[4 * FE_SIZE]);
bool libff_generate_wrong_g2_pair(uint8_t data[2 * STRIDE_SIZE]);
