#include <test/statetest/statetest.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  using namespace evmone::test;

  std::string input{reinterpret_cast<const char*>(data), size};
  std::istringstream input_stream{input};

  try {
    const auto state_tests = load_state_tests(input_stream);
    if (state_tests.empty())
      return -1;
  } catch (const json::json::exception&) {
    return -1;
  }
  return 0;
}
