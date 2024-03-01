#include "impl.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size % STRIDE_SIZE != 0)
    return -1; // unwanted

  libff_pairing_verify({data, size});
  return 0;
}
