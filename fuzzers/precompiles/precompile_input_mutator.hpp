#pragma once
#include <random>
#include <test/state/precompiles.hpp>

using evmone::state::PrecompileId;

[[nodiscard]] size_t mutate_precompile_input(std::minstd_rand& rand,
                                             PrecompileId id, uint8_t* data,
                                             size_t size, size_t max_size);
