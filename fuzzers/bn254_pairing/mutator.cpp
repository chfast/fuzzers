#include "impl.hpp"
#include <cassert>
#include <cstring>
#include <random>

// Experimental, may go away in the future.
// libFuzzer-provided function to be used inside LLVMFuzzerCustomMutator.
// Mutates raw data in [data, data+size) inplace.
// Returns the new size, which is not greater than max_size.
extern "C" size_t LLVMFuzzerMutate(uint8_t* data, size_t size, size_t max_size);

namespace {
class PairingMutator {
  uint8_t* data_;
  size_t size_;
  size_t max_size_;
  std::minstd_rand rand_;

  void expand_data(size_t added_size) {
    assert(size_ + added_size <= max_size_);
    size_t s = 0;
    do {
      s = LLVMFuzzerMutate(data_ + size_, s, max_size_ - size_);
    } while (s < added_size);
    size_ += added_size;
  }

  size_t generate_abc() {
    // std::cerr << "abc:\n";
    assert(size_ % PAIR_SIZE == 0);

    if (size_ + 2 * PAIR_SIZE > max_size_)
      return 0;

    auto init_size = size_;
    auto begin = data_ + size_;

    // we need two random scalars a and b, use existing data for this.
    expand_data(2 * FE_SIZE);
    libff_generate_abc(begin);

    auto new_size = init_size + 2 * PAIR_SIZE;
    // std::cerr << "abc: " << new_size << "\n";
    return new_size;
  }

  size_t generate_abcd() {
    assert(size_ % PAIR_SIZE == 0);

    if (size_ + 2 * PAIR_SIZE > max_size_)
      return 0;

    auto init_size = size_;
    auto begin = data_ + size_;

    // we need 3 random scalars, use the existing data for this.
    expand_data(3 * FE_SIZE);
    assert(size_ == init_size + 3 * FE_SIZE);
    libff_generate_abcd(begin);

    auto new_size = init_size + 2 * PAIR_SIZE;
    // std::cerr << "abcd: " << new_size << "\n";
    return new_size;
  }

  size_t generate_wrong_g2_pair() {
    assert(size_ % PAIR_SIZE == 0);

    if (size_ + 2 * PAIR_SIZE > max_size_)
      return 0;

    auto init_size = size_;
    auto begin = data_ + size_;
    expand_data(3 * FE_SIZE);

    libff_generate_wrong_g2_pair(begin);
    return init_size + 2 * PAIR_SIZE;
  }

  size_t generate_wrong_g2() {
    if (size_ < PAIR_SIZE)
      return 0;

    const auto g2_data = &data_[2 * FE_SIZE];
    LLVMFuzzerMutate(g2_data, 4 * FE_SIZE, 4 * FE_SIZE);
    libff_generate_wrong_g2(g2_data);
    return size_;
  }

  size_t drop_stride() {
    assert(size_ % PAIR_SIZE == 0);
    const auto num_strides = size_ / PAIR_SIZE;
    if (num_strides <= 1)
      return 0;
    const auto i = rand_() % num_strides;
    return i * PAIR_SIZE;
  }

  size_t dup_stride() {
    if (size_ < PAIR_SIZE)
      return 0;

    if (size_ + PAIR_SIZE > max_size_)
      return 0;

    const auto num_strides = size_ / PAIR_SIZE;
    const auto i = rand_() % num_strides;
    std::memcpy(&data_[size_], &data_[i * PAIR_SIZE], PAIR_SIZE);

    return size_ + PAIR_SIZE;
  }

  size_t swap_stride() {
    if (size_ < 2 * PAIR_SIZE)
      return 0;

    const auto num_strides = size_ / PAIR_SIZE;
    const auto i = rand_() % (num_strides - 1) + 1;

    uint8_t tmp[PAIR_SIZE];
    std::memcpy(tmp, &data_[i * PAIR_SIZE], PAIR_SIZE);
    std::memcpy(&data_[i * PAIR_SIZE], &data_[0], PAIR_SIZE);
    std::memcpy(&data_[0], tmp, PAIR_SIZE);
    return size_;
  }

  size_t default_fuzz() {
    const auto new_size = LLVMFuzzerMutate(data_, size_, max_size_);
    return new_size / PAIR_SIZE * PAIR_SIZE;
  }

  using MutatorFn = size_t (PairingMutator::*)();

  static constexpr MutatorFn mutators[] = {
      &PairingMutator::default_fuzz,
      &PairingMutator::generate_abc,
      &PairingMutator::generate_abcd,
      &PairingMutator::generate_wrong_g2_pair,
      &PairingMutator::generate_wrong_g2,
      &PairingMutator::drop_stride,
      &PairingMutator::dup_stride,
      &PairingMutator::swap_stride,
  };

public:
  PairingMutator(uint8_t* data, size_t size, size_t max_size, uint32_t seed)
      : data_{data}, size_{size}, max_size_{max_size}, rand_{seed} {}

  size_t mutate() {
    size_ = size_ / PAIR_SIZE * PAIR_SIZE; // align the input size

    while (true) {
      const auto i = rand_() % std::size(mutators);
      const auto m = mutators[i];
      const auto r = (this->*m)();
      if (r > 0)
        return r;
    }
  }
};
} // namespace

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data, size_t size,
                                          size_t max_size, unsigned int seed) {
  assert(max_size >= PAIR_SIZE);

  return PairingMutator{data, size, max_size, seed}.mutate();
}

// Optional user-provided custom cross-over function.
// Combines pieces of data1 & data2 together into out.
// Returns the new size, which is not greater than max_out_size.
// Should produce the same mutation given the same seed.
extern "C" size_t LLVMFuzzerCustomCrossOver(const uint8_t* data1, size_t size1,
                                            const uint8_t* data2, size_t size2,
                                            uint8_t* out, size_t max_out_size,
                                            unsigned int seed) {
  const auto max_size = std::max(size1, size2);
  assert(max_out_size >= max_size); // sanity check

  // Ignore inputs of invalid length.
  if (size1 % PAIR_SIZE != 0) [[unlikely]] {
    std::memcpy(out, data2, size2);
    return size2;
  }
  if (size2 % PAIR_SIZE != 0) [[unlikely]] {
    std::memcpy(out, data1, size1);
    return size1;
  }

  // Randomly select a pair along the "common size".
  const auto common_size = std::min(size1, size2);
  std::minstd_rand rand{seed};
  const uint8_t* sources[] = {data1, data2};
  for (size_t off = 0; off < common_size; off += PAIR_SIZE) {
    const auto src = sources[(rand() % std::size(sources))];
    std::memcpy(out + off, src + off, PAIR_SIZE);
  }

  // Copy the longer tail.
  std::memcpy(out + common_size, size1 == max_size ? data1 : data2,
              max_size - common_size);
  return max_size;
}
