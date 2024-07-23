#include "impl.hpp"
#include <cassert>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size % PAIR_SIZE != 0)
    return -1; // unwanted

  const auto r_libff = libff_pairing_verify({data, size});
  const auto r_evmmax = evmmax_pairing_verify({data, size});

  switch (r_libff) {
  case Result::zero:
  case Result::one:
  case Result::invalid_input_length:
    assert(r_evmmax == r_libff);
    break;
  case Result::invalid_g1:
  case Result::invalid_g2:
  case Result::invalid_g2_subgroup:
    assert(r_evmmax == Result::invalid_g1);
    break;
  default:
    __builtin_trap();
  }
  return 0;
}
