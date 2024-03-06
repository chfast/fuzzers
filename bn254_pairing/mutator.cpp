#include "impl.hpp"
#include <cassert>
#include <cstring>

// Experimental, may go away in the future.
// libFuzzer-provided function to be used inside LLVMFuzzerCustomMutator.
// Mutates raw data in [data, data+size) inplace.
// Returns the new size, which is not greater than max_size.
extern "C" size_t LLVMFuzzerMutate(uint8_t *data, size_t size, size_t max_size);

namespace {

// https://en.wikipedia.org/wiki/Linear_congruential_generator
inline unsigned next_seed(unsigned seed) noexcept { return 0x10DCD * seed + 1; }

void mutate_builtin_b32(uint8_t *data) {
  const auto new_size = LLVMFuzzerMutate(data, 32, 32);
  assert(new_size <= 32);
}

using Mutator = long (*)(uint8_t *, size_t, size_t, unsigned int);

/// Mutates a single field element in the first stride using builtin mutator.
long mutate_fe(uint8_t *data, size_t size, size_t max_size, unsigned int seed) {
  assert(max_size >= STRIDE_SIZE);
  assert(size >= STRIDE_SIZE);
  const auto fe_index = seed % (STRIDE_SIZE / FE_SIZE);
  mutate_builtin_b32(&data[fe_index * FE_SIZE]);
  return std::max(size, max_size);
}

long swap_stride(uint8_t *data, size_t size, size_t max_size,
                 unsigned int seed) {
  if (size < 2 * STRIDE_SIZE)
    return -1;

  const auto seed_h = seed >> 16;
  const auto seed_l = static_cast<uint16_t>(seed);
  const auto num_strides = size / STRIDE_SIZE;
  const auto i = seed_l % num_strides;
  const auto j = seed_h % num_strides;

  uint8_t tmp[STRIDE_SIZE];
  std::memcpy(tmp, &data[i * STRIDE_SIZE], STRIDE_SIZE);
  std::memcpy(&data[i * STRIDE_SIZE], &data[j * STRIDE_SIZE], STRIDE_SIZE);
  std::memcpy(&data[j * STRIDE_SIZE], tmp, STRIDE_SIZE);
  return size;
}

long rm_stride(uint8_t *data, size_t size, size_t max_size, unsigned seed) {
  if (size < 2 * STRIDE_SIZE)
    return -1;

  const auto num_strides = size / STRIDE_SIZE;
  const auto i = seed % num_strides;
  std::memmove(&data[i * STRIDE_SIZE], &data[(i + 1) * STRIDE_SIZE],
               (num_strides - 1 - i) * STRIDE_SIZE);

  return size - STRIDE_SIZE;
}

long dup_stride(uint8_t *data, size_t size, size_t max_size, unsigned seed) {
  if (size + STRIDE_SIZE > max_size)
    return -1;

  const auto num_strides = size / STRIDE_SIZE;
  const auto i = seed % num_strides;
  std::memmove(&data[size], &data[i * STRIDE_SIZE], STRIDE_SIZE);

  return size + STRIDE_SIZE;
}

long rm_half_strides(uint8_t *data, size_t size, size_t max_size,
                     unsigned seed) {
  if (size >= 4 * STRIDE_SIZE && size % 2 * STRIDE_SIZE == 0)
    return -1;

  return size / 2;
}

long zero_g1(uint8_t *data, size_t size, size_t max_size, unsigned seed) {
  assert(size >= STRIDE_SIZE);
  std::memset(&data[0], 0, 2 * FE_SIZE);
  return size;
}

long zero_g2(uint8_t *data, size_t size, size_t max_size, unsigned seed) {
  assert(size >= STRIDE_SIZE);
  std::memset(&data[2 * FE_SIZE], 0, 4 * FE_SIZE);
  return size;
}

long generate_abc(uint8_t *data, size_t size, size_t max_size, unsigned seed) {
  if (max_size < 2 * STRIDE_SIZE)
    return -1;

  // we need two random scalars a and b, use existing data for this.
  LLVMFuzzerMutate(data, 2 * FE_SIZE, 2 * FE_SIZE);
  libff_generate_abc(data);
  return std::max(size, 2 * STRIDE_SIZE);
}

long generate_abcd(uint8_t *data, size_t size, size_t max_size, unsigned seed) {
  if (max_size < 2 * STRIDE_SIZE)
    return -1;

  do {
    // we need 3 random scalars, use the existing data for this.
    LLVMFuzzerMutate(data, 3 * FE_SIZE, 3 * FE_SIZE);
  } while (!libff_generate_abcd(data));
  return std::max(size, 2 * STRIDE_SIZE);
}

long generate_wrong_g2(uint8_t *data, size_t size, size_t max_size,
                       unsigned seed) {
  if (size < STRIDE_SIZE)
    return -1;

  const auto g2_data = &data[2 * FE_SIZE];
  do {
    LLVMFuzzerMutate(g2_data, 4 * FE_SIZE, 4 * FE_SIZE);
  } while (!libff_generate_wrong_g2(g2_data));
  return size;
}

long generate_wrong_g2_pair(uint8_t *data, size_t size, size_t max_size,
                            unsigned seed) {
  if (size < 2 * STRIDE_SIZE)
    return -1;

  do {
    LLVMFuzzerMutate(data, STRIDE_SIZE, STRIDE_SIZE);
  } while (!libff_generate_wrong_g2_pair(data));
  return size;
}

constexpr Mutator mutators[] = {
    mutate_fe,
    swap_stride,
    rm_stride,
    dup_stride,
    rm_half_strides,
    zero_g1,
    zero_g2,
    generate_abc,
    generate_abcd,
    generate_wrong_g2,
    generate_wrong_g2_pair,
};
} // namespace

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t *data, size_t size,
                                          size_t max_size, unsigned int seed) {
  assert(max_size >= STRIDE_SIZE);
  size = std::max(size, STRIDE_SIZE);

  while (true) {
    const auto i = seed % std::size(mutators);
    const auto m = mutators[i];
    const auto r = m(data, size, max_size, seed);
    if (r >= 0)
      return r;
    seed = next_seed(seed);
  }
}
