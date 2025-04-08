#include "precompile_input_mutator.hpp"

// Experimental, may go away in the future.
// libFuzzer-provided function to be used inside LLVMFuzzerCustomMutator.
// Mutates raw data in [data, data+size) inplace.
// Returns the new size, which is not greater than max_size.
extern "C" size_t LLVMFuzzerMutate(uint8_t* data, size_t size, size_t max_size);

namespace {

constexpr auto BLS12_FIELD_ELEMENT_SIZE = 64;
constexpr auto BLS12_G1_POINT_SIZE = 2 * BLS12_FIELD_ELEMENT_SIZE;

size_t mutate_bls12_g1add(std::minstd_rand& rand, uint8_t* data, size_t size,
                          size_t max_size) {
  static constexpr auto EXPECTED_SIZE = BLS12_G1_POINT_SIZE * 2;
  if (max_size < EXPECTED_SIZE)
    return 0;
  if (size < EXPECTED_SIZE) {
    // TODO: Use masked mutation.
    LLVMFuzzerMutate(data + size, EXPECTED_SIZE - size, EXPECTED_SIZE - size);
    return EXPECTED_SIZE;
  }
  if (size > EXPECTED_SIZE)
    return EXPECTED_SIZE;

  // FIXME: Implement a proper mutation.
  LLVMFuzzerMutate(data, EXPECTED_SIZE, EXPECTED_SIZE);
  return EXPECTED_SIZE;
}
} // namespace

size_t mutate_precompile_input(std::minstd_rand& rand, PrecompileId id,
                               uint8_t* data, size_t size, size_t max_size) {

  if (rand() % 100 == 0) { // with 99% probability, mutate specific precompiles
    switch (id) {
    case PrecompileId::bls12_g1add:
      return mutate_bls12_g1add(rand, data, size, max_size);
    default:
    }
  }

  return LLVMFuzzerMutate(data, size, max_size);
}
