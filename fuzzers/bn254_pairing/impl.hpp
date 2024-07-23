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

inline constexpr size_t FE_SIZE = 32;                  ///< Field element size.
inline constexpr size_t G1_SIZE = 2 * FE_SIZE;         ///< G1 point size.
inline constexpr size_t G2_SIZE = 2 * G1_SIZE;         ///< G2 point size.
inline constexpr size_t PAIR_SIZE = G1_SIZE + G2_SIZE; ///< Pair size.

Result libff_pairing_verify(bytes_view input) noexcept;

void libff_generate_abc(uint8_t data[2 * PAIR_SIZE]);
void libff_generate_abcd(uint8_t data[2 * PAIR_SIZE]);
void libff_generate_wrong_g2(uint8_t data[G2_SIZE]);
void libff_generate_wrong_g2_pair(uint8_t data[2 * PAIR_SIZE]);

Result evmmax_pairing_verify(bytes_view input) noexcept;
