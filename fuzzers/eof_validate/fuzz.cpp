#include "eof_validate.hpp"
#include <besu_fzz.hpp>
#include <geth_fzz.h>
#include <iostream>
#include <revm_fzz.hpp>
#include <test/utils/bytecode.hpp>

using namespace evmone;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data_ptr,
                                      size_t data_size) noexcept {
  // TODO: Add option to disable external projects, e.g. -besu=0.
  // TODO: Compare error categories.

  using namespace fzz;

  // FIXME: Fix evmone API to properly handle inputs above the limit.
  if (data_size > 0xc000)
    return -1;
  const bytes_view data{data_ptr, data_size};

  const auto vh = validate_header(REV, data);
  const auto v_status = validate_eof(REV, ContainerKind::runtime, data);
  assert(v_status != EOFValidationError::impossible);

  const auto initcode_status = validate_eof(REV, ContainerKind::initcode, data);

  // if (v_status != EOFValidationError::success) {
  //   // Inspect found categories.
  //   const auto cat = get_cat(v_status);
  //   if (cat == EOFErrCat::other) {
  //     std::cerr << v_status << "\n";
  //     std::abort();
  //   }
  // }

  const auto evm1_ok = v_status == EOFValidationError::success;
  const auto evm1_ik = initcode_status == EOFValidationError::success;
  const auto evm1_v2 = (evm1_ik << 1) | evm1_ok;

  switch (v_status) {
  // case EOFValidationError::invalid_non_returning_flag: // incorrect
  // case EOFValidationError::success:                    // incorrect
  // break;
  default: {
    // std::cerr << "XXXX " << v_status << "\n";
    const auto revm_v2 = fzz_revm_validate_eof(data_ptr, data_size);
    if (revm_v2 != evm1_v2) {
      std::cerr << "evm1: " << v_status << " " << initcode_status << "\n"
                << "revm: " << revm_v2 << "\n"
                << "code: " << hex(data) << "\n"
                << "size: " << data.size() << "\n";
      std::abort();
    }
  }
  }

  switch (v_status) {
  // case EOFValidationError::incompatible_container_kind:
  // break;
  default: {
    // std::cerr << "XXXX " << v_status << "\n";
    const auto besu_v2 = fzz_besu_validate_eof(data_ptr, data_size);
    if (besu_v2 != evm1_v2) {
      std::cerr << "evm1: " << v_status << " " << initcode_status << "\n"
                << "besu: " << besu_v2 << "\n"
                << "code: " << hex(data) << "\n"
                << "size: " << data.size() << "\n";
      std::abort();
    }
  }
  }

  switch (v_status) {
  // case EOFValidationError::incompatible_container_kind:
  // break;
  default: {
    // std::cerr << "XXXX " << v_status << "\n";
    const auto geth_v2 = geth_fzz_eof_validate(const_cast<uint8_t*>(data_ptr),
                                               static_cast<GoInt>(data_size));
    if (geth_v2 != evm1_v2) {
      std::cerr << "evm1: " << v_status << " " << initcode_status << "\n"
                << "geth: " << geth_v2 << "\n"
                << "code: " << hex(data) << "\n"
                << "size: " << data.size() << "\n";
      std::abort();
    }
  }
  }

  const auto p_vh_status = std::get_if<EOFValidationError>(&vh);
  assert(p_vh_status == nullptr || *p_vh_status != EOFValidationError::success);
  const auto vh_status =
      (p_vh_status != nullptr) ? *p_vh_status : EOFValidationError::success;
  assert(vh_status != EOFValidationError::impossible);

  if (v_status == EOFValidationError::success &&
      vh_status != EOFValidationError::success)
    __builtin_trap();

  // TODO: bytecode testing disabled.
  // if (v_status == EOFValidationError::success) {
  //   const auto h = read_valid_eof1_header(data);
  //
  //   test::eof_bytecode bc{bytes{h.get_code(data, 0)},
  //                         h.types[0].max_stack_height};
  //   bc.data(bytes{h.get_data(data)});
  //
  //   for (size_t i = 1; i < h.code_sizes.size(); ++i)
  //     bc.code(bytes{h.get_code(data, i)}, h.types[i].inputs,
  //     h.types[i].outputs,
  //             h.types[i].max_stack_height);
  //
  //   for (size_t i = 0; i < h.container_sizes.size(); ++i)
  //     bc.container(bytes{h.get_container(data, i)});
  //
  //   const auto serialized = test::bytecode{bc};
  //   if (serialized != data) {
  //     std::cerr << "input: " << hex(data) << "\n";
  //     std::cerr << "bc:    " << hex(serialized) << "\n";
  //
  //     __builtin_trap();
  //   }
  // }
  return 0;
}
