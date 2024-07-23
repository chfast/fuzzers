#include "impl.hpp"

#include <evmone_precompiles/bn254.hpp>
#include <intx/intx.hpp>

namespace {
Result _ecpairing_execute(const uint8_t* input, size_t input_size) noexcept {
  const auto pair_size = 192;

  if (input_size % pair_size != 0)
    return Result::invalid_input_length;

  const auto pair_count = input_size / pair_size;

  if (pair_count > 0) {
    auto input_idx = input;

    std::vector<evmmax::bn254::Point> vG1(pair_count);
    std::vector<evmmax::bn254::ExtPoint> vG2(pair_count);

    for (size_t i = 0; i < pair_count; ++i) {
      const evmmax::bn254::Point p = {
          intx::be::unsafe::load<intx::uint256>(input_idx),
          intx::be::unsafe::load<intx::uint256>(input_idx + 32),
      };

      const evmmax::bn254::ExtPoint q = {
          {intx::be::unsafe::load<intx::uint256>(input_idx + 96),
           intx::be::unsafe::load<intx::uint256>(input_idx + 64)},
          {intx::be::unsafe::load<intx::uint256>(input_idx + 160),
           intx::be::unsafe::load<intx::uint256>(input_idx + 128)},
      };

      vG1[i] = p;
      vG2[i] = q;

      input_idx += pair_size;
    }

    const auto res = evmmax::bn254::pairing(vG2, vG1);

    if (res.has_value()) {
      return res.value() ? Result::one : Result::zero;
    } else
      return Result::invalid_g1;
  } else {
    return Result::one;
  }
}

} // namespace

Result evmmax_pairing_verify(bytes_view input) noexcept {
  return _ecpairing_execute(input.data(), input.size());
}
