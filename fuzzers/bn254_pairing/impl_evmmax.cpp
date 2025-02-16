#include "impl.hpp"

#include <evmone_precompiles/bn254.hpp>
#include <intx/intx.hpp>

namespace {
Result _ecpairing_execute(const uint8_t* input, size_t input_size) noexcept {
  static constexpr size_t PAIR_SIZE = 192;

  if (input_size % PAIR_SIZE != 0)
    return Result::invalid_input_length;

  if (const auto n_pairs = input_size / PAIR_SIZE; n_pairs > 0) {
    std::vector<std::pair<evmmax::bn254::Point, evmmax::bn254::ExtPoint>> pairs;
    pairs.reserve(n_pairs);
    auto input_idx = input;
    for (size_t i = 0; i < n_pairs; ++i) {
      const evmmax::bn254::Point p{
          intx::be::unsafe::load<intx::uint256>(input_idx),
          intx::be::unsafe::load<intx::uint256>(input_idx + 32),
      };

      const evmmax::bn254::ExtPoint q{
          {intx::be::unsafe::load<intx::uint256>(input_idx + 96),
           intx::be::unsafe::load<intx::uint256>(input_idx + 64)},
          {intx::be::unsafe::load<intx::uint256>(input_idx + 160),
           intx::be::unsafe::load<intx::uint256>(input_idx + 128)},
      };
      pairs.emplace_back(p, q);
      input_idx += PAIR_SIZE;
    }

    const auto res = evmmax::bn254::pairing_check(pairs);

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
