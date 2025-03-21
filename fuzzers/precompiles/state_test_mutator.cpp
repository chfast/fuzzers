#include "common.hpp"
#include <cassert>
#include <cstring>
#include <evmone/evmone.h>
#include <random>
#include <test/state/mpt_hash.hpp>
#include <test/statetest/statetest.hpp>
#include <test/utils/bytecode.hpp>

// Experimental, may go away in the future.
// libFuzzer-provided function to be used inside LLVMFuzzerCustomMutator.
// Mutates raw data in [data, data+size) inplace.
// Returns the new size, which is not greater than max_size.
extern "C" size_t LLVMFuzzerMutate(uint8_t* data, size_t size, size_t max_size);

// namespace {
// class StateTestMutator {
//   uint8_t* data_;
//   size_t size_;
//   size_t max_size_;
//   std::minstd_rand rand_;
//
//   void expand_data(size_t added_size) {
//     assert(size_ + added_size <= max_size_);
//     size_t s = 0;
//     do {
//       s = LLVMFuzzerMutate(data_ + size_, s, max_size_ - size_);
//     } while (s < added_size);
//     size_ += added_size;
//   }
//
//   size_t generate_abc() {
//     // std::cerr << "abc:\n";
//     assert(size_ % PAIR_SIZE == 0);
//
//     if (size_ + 2 * PAIR_SIZE > max_size_)
//       return 0;
//
//     auto init_size = size_;
//     auto begin = data_ + size_;
//
//     // we need two random scalars a and b, use existing data for this.
//     expand_data(2 * FE_SIZE);
//     libff_generate_abc(begin);
//
//     auto new_size = init_size + 2 * PAIR_SIZE;
//     // std::cerr << "abc: " << new_size << "\n";
//     return new_size;
//   }
//
//   size_t generate_abcd() {
//     assert(size_ % PAIR_SIZE == 0);
//
//     if (size_ + 2 * PAIR_SIZE > max_size_)
//       return 0;
//
//     auto init_size = size_;
//     auto begin = data_ + size_;
//
//     // we need 3 random scalars, use the existing data for this.
//     expand_data(3 * FE_SIZE);
//     assert(size_ == init_size + 3 * FE_SIZE);
//     libff_generate_abcd(begin);
//
//     auto new_size = init_size + 2 * PAIR_SIZE;
//     // std::cerr << "abcd: " << new_size << "\n";
//     return new_size;
//   }
//
//   size_t generate_wrong_g2_pair() {
//     assert(size_ % PAIR_SIZE == 0);
//
//     if (size_ + 2 * PAIR_SIZE > max_size_)
//       return 0;
//
//     auto init_size = size_;
//     auto begin = data_ + size_;
//     expand_data(3 * FE_SIZE);
//
//     libff_generate_wrong_g2_pair(begin);
//     return init_size + 2 * PAIR_SIZE;
//   }
//
//   size_t generate_wrong_g2() {
//     if (size_ < PAIR_SIZE)
//       return 0;
//
//     const auto g2_data = &data_[2 * FE_SIZE];
//     LLVMFuzzerMutate(g2_data, 4 * FE_SIZE, 4 * FE_SIZE);
//     libff_generate_wrong_g2(g2_data);
//     return size_;
//   }
//
//   size_t drop_stride() {
//     assert(size_ % PAIR_SIZE == 0);
//     const auto num_strides = size_ / PAIR_SIZE;
//     if (num_strides <= 1)
//       return 0;
//     const auto i = rand_() % num_strides;
//     return i * PAIR_SIZE;
//   }
//
//   size_t dup_stride() {
//     if (size_ < PAIR_SIZE)
//       return 0;
//
//     if (size_ + PAIR_SIZE > max_size_)
//       return 0;
//
//     const auto num_strides = size_ / PAIR_SIZE;
//     const auto i = rand_() % num_strides;
//     std::memcpy(&data_[size_], &data_[i * PAIR_SIZE], PAIR_SIZE);
//
//     return size_ + PAIR_SIZE;
//   }
//
//   size_t swap_stride() {
//     if (size_ < 2 * PAIR_SIZE)
//       return 0;
//
//     const auto num_strides = size_ / PAIR_SIZE;
//     const auto i = rand_() % (num_strides - 1) + 1;
//
//     uint8_t tmp[PAIR_SIZE];
//     std::memcpy(tmp, &data_[i * PAIR_SIZE], PAIR_SIZE);
//     std::memcpy(&data_[i * PAIR_SIZE], &data_[0], PAIR_SIZE);
//     std::memcpy(&data_[0], tmp, PAIR_SIZE);
//     return size_;
//   }
//
//   size_t default_fuzz() {
//     const auto new_size = LLVMFuzzerMutate(data_, size_, max_size_);
//     return new_size / PAIR_SIZE * PAIR_SIZE;
//   }
//
//   using MutatorFn = size_t (PairingMutator::*)();
//
//   static constexpr MutatorFn mutators[] = {
//       &PairingMutator::default_fuzz,
//       &PairingMutator::generate_abc,
//       &PairingMutator::generate_abcd,
//       &PairingMutator::generate_wrong_g2_pair,
//       &PairingMutator::generate_wrong_g2,
//       &PairingMutator::drop_stride,
//       &PairingMutator::dup_stride,
//       &PairingMutator::swap_stride,
//   };
//
// public:
//   PairingMutator(uint8_t* data, size_t size, size_t max_size, uint32_t seed)
//       : data_{data}, size_{size}, max_size_{max_size}, rand_{seed} {}
//
//   size_t mutate() {
//     size_ = size_ / PAIR_SIZE * PAIR_SIZE; // align the input size
//
//     while (true) {
//       const auto i = rand_() % std::size(mutators);
//       const auto m = mutators[i];
//       const auto r = (this->*m)();
//       if (r > 0)
//         return r;
//     }
//   }
// };
// } // namespace

using namespace evmc::literals;
using namespace evmone::test;

namespace {
evmc::VM vm{evmc_create_evmone()};

constexpr auto REV = EVMC_PRAGUE;
constexpr auto SENDER = 0xe100713FC15400D1e94096a545879E7c6407001e_address;
constexpr auto BASEFEE = 10;
constexpr auto GAS_LIMIT = 1'000'000;
constexpr auto PRECOMPILE_PROXY = 0x00097ec03911e0097087_address;

const auto precompile_proxy_code = [] {
  const auto store_loop_head = 29;
  const auto store_loop_body = 37;
  auto code =
      bytecode() +                                      //
      OP_PUSH0 + OP_PUSH0 + OP_CALLDATASIZE +           // [input_size, 0, 0]
      OP_DUP1 + OP_PUSH0 + OP_PUSH0 + OP_CALLDATACOPY + // [input_size, 0, 0]
      OP_PUSH0 + OP_CALLVALUE + OP_GAS + // [gas, addr, 0, input_size, 0, 0]
      OP_STATICCALL +                    // [return_code]
      sstore(1) +                        // [] store the return code @ 1.
      OP_RETURNDATASIZE +                // [output_size]
      OP_DUP1 + OP_PUSH0 + OP_PUSH0 + OP_RETURNDATACOPY + // [output_size]
      OP_PUSH0 + OP_DUP2 +
      OP_MSTORE +           // [output_size]  clear 32 bytes after the output.
      OP_DUP1 + sstore(2) + // [output_size]  store the output size @ 2.
      push(32) +            // [32, output_size]
      OP_PUSH0 +            // [off=0, 32, output_size]
      OP_JUMPDEST +         // @store-loop-head
      OP_DUP3 + OP_DUP2 + OP_LT +  // [off < output_size, off, 32, output_size]
      store_loop_body + OP_JUMPI + // [off, 32, output_size] → @store-loop-body
      OP_STOP +                    //
      OP_JUMPDEST +                // @store-loop-body
      OP_DUP1 + OP_MLOAD +         // [output[off], off, 32, output_size]
      OP_DUP2 + OP_SSTORE + // [off, 32, output_size] store output[off] @ off.
      OP_DUP2 + OP_ADD +    // [off+=32, 32, output_size]
      store_loop_head + OP_JUMP; // [off, 32, output_size] → @store-loop-head
  return bytes{code};
}();

StateTransitionTest build_minimal_precompile_proxy_test() {
  StateTransitionTest test;
  auto& c = test.cases.emplace_back();
  c.block.number = 1;
  c.block.gas_limit = GAS_LIMIT;
  c.block.base_fee = BASEFEE;
  c.rev = REV;
  auto& m = test.multi_tx;
  m.gas_limits.emplace_back(c.block.gas_limit);
  m.inputs.emplace_back();
  m.values.emplace_back(1);
  auto& e = c.expectations.emplace_back();
  m.sender = SENDER;
  m.max_gas_price = c.block.base_fee;
  m.max_priority_gas_price = c.block.base_fee;

  test.pre_state[SENDER] = {
      .balance = 10'000'000'000,
  };

  return test;
}
} // namespace

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data, size_t size,
                                          size_t max_size, unsigned int seed) {
  assert(size != 0);

  std::minstd_rand rand_{seed};

  std::string input{reinterpret_cast<const char*>(data), size};
  std::istringstream input_stream{input};

  std::optional<StateTransitionTest> test;
  if (size > 1) {
    test = fzz::load_state_test(input_stream);
  }
  if (!test)
    test = build_minimal_precompile_proxy_test();

  // Setup precompile proxy test.
  auto& precompile_proxy = test->pre_state[PRECOMPILE_PROXY];
  precompile_proxy.code = precompile_proxy_code;
  test->multi_tx.to = PRECOMPILE_PROXY;

  if (rand_() % 100 < 1) {
    // Mutate the precompile id.
    const auto id = test->multi_tx.values[0][0];
    const auto new_id = (id + rand_()) % 0x13;
    test->multi_tx.values[0][0] = new_id;
  } else if (rand_() % 100 < 2) {
    // Mutate the precompile gas limit.
    const auto gas_limit = test->multi_tx.gas_limits[0];
    const auto new_gas_limit =
        (gas_limit + static_cast<int64_t>(rand_() % 1000 - 500)) %
        test->cases[0].block.gas_limit;
    test->multi_tx.gas_limits[0] = new_gas_limit;
  } else { // Mutate the precompile input.
    auto& calldata = test->multi_tx.inputs[0];
    bytes calldata_copy = calldata;

    // calldata will be hex encoded, so we can extend it by the half of
    // available space.
    const auto max_calldata_size = calldata.size() + (max_size - size) / 2;
    assert(max_calldata_size >= calldata.size());
    assert(max_calldata_size < max_size);
    calldata_copy.resize(max_calldata_size);
    const auto new_size = LLVMFuzzerMutate(calldata_copy.data(),
                                           calldata.size(), max_calldata_size);
    calldata_copy.resize(new_size);
    calldata = calldata_copy;
  }

  // Execute state: we need the result so that the test file is ready to go.
  // TODO: This should be taken off the mutation.

  auto& c = test->cases[0];
  auto tx = test->multi_tx.get(c.expectations[0].indexes);
  const auto& [rev, cases, block] = test->cases[0];
  // auto state = test->pre_state;
  //
  // const auto res = transition(
  //     state, block, test->block_hashes, tx, rev, vm, block.gas_limit,
  //     static_cast<int64_t>(evmone::state::max_blob_gas_per_block(rev)));
  //
  // // Finalize block with reward 0.
  // finalize(state, rev, block.coinbase, 0, {}, {});

  // Save the test.
  auto j = to_state_test("", c.block, tx, test->pre_state, c.rev, {}, {});
  std::ostringstream output_stream;
  output_stream << std::setw(2) << j;
  const auto output = output_stream.str();
  if (output.size() > max_size)
    return 0;

  std::memcpy(data, output.data(), output.size());
  return output.size();
}

// Optional user-provided custom cross-over function.
// Combines pieces of data1 & data2 together into out.
// Returns the new size, which is not greater than max_out_size.
// Should produce the same mutation given the same seed.
// extern "C" size_t LLVMFuzzerCustomCrossOver(const uint8_t* data1, size_t
// size1,
//                                             const uint8_t* data2, size_t
//                                             size2, uint8_t* out, size_t
//                                             max_out_size, unsigned int seed)
//                                             {
//   const auto max_size = std::max(size1, size2);
//   assert(max_out_size >= max_size); // sanity check
//
//   // Ignore inputs of invalid length.
//   if (size1 % PAIR_SIZE != 0) [[unlikely]] {
//     std::memcpy(out, data2, size2);
//     return size2;
//   }
//   if (size2 % PAIR_SIZE != 0) [[unlikely]] {
//     std::memcpy(out, data1, size1);
//     return size1;
//   }
//
//   // Randomly select a pair along the "common size".
//   const auto common_size = std::min(size1, size2);
//   std::minstd_rand rand{seed};
//   const uint8_t* sources[] = {data1, data2};
//   for (size_t off = 0; off < common_size; off += PAIR_SIZE) {
//     const auto src = sources[(rand() % std::size(sources))];
//     std::memcpy(out + off, src + off, PAIR_SIZE);
//   }
//
//   // Copy the longer tail.
//   std::memcpy(out + common_size, size1 == max_size ? data1 : data2,
//               max_size - common_size);
//   return max_size;
// }
