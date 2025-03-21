#include <evmone/evmone.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <test/statetest/statetest.hpp>

namespace fs = std::filesystem;
using namespace evmone::test;

static evmc::VM vm{evmc_create_evmone()};

static void run_state_test(const StateTransitionTest& test, evmc::VM& vm) {
  for (const auto& [rev, cases, block] : test.cases) {
    validate_state(test.pre_state, rev);
    for (size_t case_index = 0; case_index != cases.size(); ++case_index) {
      const auto& expected = cases[case_index];
      const auto tx = test.multi_tx.get(expected.indexes);
      auto state = test.pre_state;

      const auto res = transition(
          state, block, test.block_hashes, tx, rev, vm, block.gas_limit,
          static_cast<int64_t>(evmone::state::max_blob_gas_per_block(rev)));

      // Finalize block with reward 0.
      finalize(state, rev, block.coinbase, 0, {}, {});

      // const auto state_root = state::mpt_hash(state);
      //
      // if (expected.exception)
      // {
      //     ASSERT_FALSE(holds_alternative<state::TransactionReceipt>(res))
      //         << "unexpected valid transaction";
      //     EXPECT_EQ(logs_hash(std::vector<state::Log>()),
      //     expected.logs_hash);
      // }
      // else
      // {
      //     ASSERT_TRUE(holds_alternative<state::TransactionReceipt>(res))
      //         << "unexpected invalid transaction: " <<
      //         get<std::error_code>(res).message();
      //     EXPECT_EQ(logs_hash(get<state::TransactionReceipt>(res).logs),
      //     expected.logs_hash);
      // }
      //
      // EXPECT_EQ(state_root, expected.state_hash);
    }
  }
}

namespace {

std::string export_test(std::string input) { return input; }

void export_corpus(std::string_view extension, const fs::path& corpus_dir,
                   const fs::path& out_dir) {
  for (const auto& entry : fs::directory_iterator{corpus_dir}) {
    if (entry.is_regular_file()) {

      std::ifstream in{entry.path(), std::ios::binary};
      std::string content((std::istreambuf_iterator<char>(in)),
                          std::istreambuf_iterator<char>());
      std::string exported = export_test(content);

      auto filename = entry.path().filename();
      if (!filename.has_extension()) {
        filename += extension;
      }
      const auto out_path = out_dir / filename;
      std::ofstream out{out_path, std::ios::binary};
      out << exported;
      if (!out) {
        std::cerr << "Failed to write to " << out_path << "\n";
      }
    }
  }
}
} // namespace

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

extern "C" int LLVMFuzzerInitialize(const int* argc_ptr,
                                    const char*** argv_ptr) {
  const auto argc = *argc_ptr;
  const auto argv = *argv_ptr;

  if (argc >= 2 && std::string_view{argv[1]} == "export") {
    if (argc < 4) {
      std::cerr << "Usage: " << argv[0] << " export corpus_dir out_dir\n";
      std::exit(1);
    }
    const fs::path corpus_dir{argv[2]};
    const fs::path out_dir{argv[3]};

    if (!fs::exists(corpus_dir) || !fs::is_directory(corpus_dir)) {
      std::cerr << "Invalid corpus directory: " << corpus_dir << "\n";
      std::exit(1);
    }

    if (!fs::exists(out_dir)) {
      if (!fs::create_directories(out_dir)) {
        std::cerr << "Failed to create output directory: " << out_dir << "\n";
        std::exit(1);
      }
    }

    export_corpus(".json", corpus_dir, out_dir);
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
