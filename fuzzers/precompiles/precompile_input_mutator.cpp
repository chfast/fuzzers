#include "precompile_input_mutator.hpp"

// Experimental, may go away in the future.
// libFuzzer-provided function to be used inside LLVMFuzzerCustomMutator.
// Mutates raw data in [data, data+size) inplace.
// Returns the new size, which is not greater than max_size.
extern "C" size_t LLVMFuzzerMutate(uint8_t* data, size_t size, size_t max_size);

size_t mutate_precompile_input(std::minstd_rand& rand, PrecompileId id,
                               uint8_t* data, size_t size, size_t max_size) {
  return LLVMFuzzerMutate(data, size, max_size);
}
