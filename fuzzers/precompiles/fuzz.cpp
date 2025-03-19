#include <test/statetest/statetest.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  using namespace evmone::test;

  std::string input{reinterpret_cast<const char*>(data), size};
  std::istringstream input_stream{input};
  const auto state_tests = load_state_tests(input_stream);
  assert(state_tests.size() == 1);

  return 0;
}
