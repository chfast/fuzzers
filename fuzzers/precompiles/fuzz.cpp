#include <evmone/evmone.h>
#include <iostream>
#include <test/statetest/statetest.hpp>

using namespace evmone::test;

static evmc::VM vm{evmc_create_evmone()};

static void run_state_test(const StateTransitionTest& test, evmc::VM& vm)
{
    for (const auto& [rev, cases, block] : test.cases)
    {
        validate_state(test.pre_state, rev);
        for (size_t case_index = 0; case_index != cases.size(); ++case_index)
        {
            const auto& expected = cases[case_index];
            const auto tx = test.multi_tx.get(expected.indexes);
            auto state = test.pre_state;

            const auto res = transition(state, block, test.block_hashes, tx, rev, vm,
                block.gas_limit, static_cast<int64_t>(
                    evmone::state::max_blob_gas_per_block(rev)));

            // Finalize block with reward 0.
            finalize(state, rev, block.coinbase, 0, {}, {});

            // const auto state_root = state::mpt_hash(state);
            //
            // if (expected.exception)
            // {
            //     ASSERT_FALSE(holds_alternative<state::TransactionReceipt>(res))
            //         << "unexpected valid transaction";
            //     EXPECT_EQ(logs_hash(std::vector<state::Log>()), expected.logs_hash);
            // }
            // else
            // {
            //     ASSERT_TRUE(holds_alternative<state::TransactionReceipt>(res))
            //         << "unexpected invalid transaction: " << get<std::error_code>(res).message();
            //     EXPECT_EQ(logs_hash(get<state::TransactionReceipt>(res).logs), expected.logs_hash);
            // }
            //
            // EXPECT_EQ(state_root, expected.state_hash);
        }
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::string input{reinterpret_cast<const char*>(data), size};
  std::istringstream input_stream{input};

  try {
    const auto state_tests = load_state_tests(input_stream);
    if (state_tests.empty())
      return -1;

    run_state_test(state_tests[0], vm);

  } catch (const json::json::exception&) {
    return -1;
  }



  return 0;
}

extern "C" int LLVMFuzzerInitialize(const int *argc_ptr, const char ***argv_ptr) {
  const auto argc = *argc_ptr;
  const auto argv = *argv_ptr;

  if (argc >= 2 && std::string_view{argv[1]} == "export") {
    if (argc < 4) {
      std::cerr << "Usage: " << argv[0] << " export corpus_dir out_dir\n";
      std::exit(1);
    }
    std::exit(0);
  }

  // for (int i = 0; i < argc; ++i) {
  //   std::string_view arg{argv[i]};
  //   if (arg == "export")
  // }

  std::cout << "LLVMFuzzerInitialize called" << std::endl;

  // std::exit(0);
  return 0;
}
