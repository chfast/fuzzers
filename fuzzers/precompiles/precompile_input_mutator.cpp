#include "precompile_input_mutator.hpp"

#include <blst.h>

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

  if (size > EXPECTED_SIZE) {
    // FIXME: This early return doesn't work for mutation.
    // return EXPECTED_SIZE;
    size = EXPECTED_SIZE;
  }

  if (std::count(data, data + 16, 0) != 16) {
    std::memset(data, 0, 16);
    return EXPECTED_SIZE;
  }
  if (std::count(data + 64, data + 64 + 16, 0) != 16) {
    std::memset(data + 64, 0, 16);
    return EXPECTED_SIZE;
  }
  if (std::count(data + 128, data + 128 + 16, 0) != 16) {
    std::memset(data + 128, 0, 16);
    return EXPECTED_SIZE;
  }
  if (std::count(data + 192, data + 192 + 16, 0) != 16) {
    std::memset(data + 192, 0, 16);
    return EXPECTED_SIZE;
  }

  blst_fp x0, y0, x1, y1;
  blst_fp_from_bendian(&x0, data + 16);
  blst_fp_from_bendian(&y0, data + 16 + 64);
  blst_fp_from_bendian(&x1, data + 16 + 128);
  blst_fp_from_bendian(&y1, data + 16 + 192);

  blst_p1_affine p{x0, y0};
  blst_p1_affine q{x1, y1};

  const auto s = rand() % 2 == 0 ? &p : &q;

  if (rand() % 2 == 0) {
    // +1
    blst_p1 r;
    blst_p1_add_or_double_affine(&r, blst_p1_generator(), s);
    blst_p1_to_affine(s, &r);
  } else {
    // mess up the x0
    LLVMFuzzerMutate(data + 16, 48, 48);
    data[16] |= 0x80; // add the expected compression bit

    // use uncompress function to create a point from random x coordinate
    // this reports errors like "not on curve" but ignore these
    // it looks it modifies the output point despite errors
    blst_p1_uncompress(s, data + 16);
  }

  const auto out = data + (rand() % 2 == 0 ? 0 : BLS12_G1_POINT_SIZE);
  blst_bendian_from_fp(out + 16, &s->x);
  blst_bendian_from_fp(out + 16 + 64, &s->y);
  return EXPECTED_SIZE;
}
} // namespace

size_t mutate_precompile_input(std::minstd_rand& rand, PrecompileId id,
                               uint8_t* data, size_t size, size_t max_size) {

  if (rand() % 100 != 0) { // with 99% probability, mutate specific precompiles
    switch (id) {
    case PrecompileId::bls12_g1add:
      return mutate_bls12_g1add(rand, data, size, max_size);
    default:
    }
  }

  return LLVMFuzzerMutate(data, size, max_size);
}
