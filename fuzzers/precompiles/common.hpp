#pragma once
#include <test/statetest/statetest.hpp>

namespace fzz {
using namespace evmone::test;
std::optional<StateTransitionTest> load_state_test(std::istream& input);
} // namespace fzz
