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
  assert(max_size >= INPUT_STRIDE);
  // assert(size >= INPUT_STRIDE);
  const auto fe_index = seed % FE_SIZE;
  mutate_builtin_b32(&data[fe_index * FE_SIZE]);
  return std::max(size, max_size);
}

long swap_stride(uint8_t *data, size_t size, size_t max_size,
                 unsigned int seed) {
  if (size < 2 * INPUT_STRIDE)
    return -1;

  const auto seed_h = seed >> 16;
  const auto seed_l = static_cast<uint16_t>(seed);
  const auto num_strides = size / INPUT_STRIDE;
  const auto i = seed_l % num_strides;
  const auto j = seed_h % num_strides;

  uint8_t tmp[INPUT_STRIDE];
  std::memcpy(tmp, &data[i * INPUT_STRIDE], INPUT_STRIDE);
  std::memcpy(&data[i * INPUT_STRIDE], &data[j * INPUT_STRIDE], INPUT_STRIDE);
  std::memcpy(&data[j * INPUT_STRIDE], tmp, INPUT_STRIDE);
  return size;
}

long rm_stride(uint8_t *data, size_t size, size_t max_size, unsigned seed) {
  if (size < 2 * INPUT_STRIDE)
    return -1;

  const auto num_strides = size / INPUT_STRIDE;
  const auto i = seed % num_strides;
  std::memmove(&data[i * INPUT_STRIDE], &data[(i + 1) * INPUT_STRIDE],
               (num_strides - 1 - i) * INPUT_STRIDE);

  return size - INPUT_STRIDE;
}

constexpr Mutator mutators[] = {mutate_fe, swap_stride, rm_stride};
} // namespace

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t *data, size_t size,
                                          size_t max_size, unsigned int seed) {
  assert(max_size >= INPUT_STRIDE);
  size = std::max(size, INPUT_STRIDE);

  while (true) {
    const auto i = seed % std::size(mutators);
    const auto m = mutators[i];
    const auto r = m(data, size, max_size, seed);
    if (r >= 0)
      return r;
    seed = next_seed(seed);
  }
}
