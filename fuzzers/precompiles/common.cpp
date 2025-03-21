#include "common.hpp"

namespace fzz {
std::optional<StateTransitionTest> load_state_test(std::istream& input) {
  std::vector<StateTransitionTest> tests;
  try {
    tests = load_state_tests(input);
  } catch (const json::json::exception&) {
    return std::nullopt;
  }
  if (tests.empty())
    return std::nullopt;

  // FIXME: Handle files with multiple tests.
  assert(tests.size() == 1);
  return tests[0];
}
}
