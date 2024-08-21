#include "eof_validate.hpp"
#include <besu_fzz.hpp>
#include <iostream>
#include <revm_fzz.hpp>
#include <test/utils/bytecode.hpp>

using namespace evmone;

namespace fzz {
namespace {

[[maybe_unused]] EOFValidationError
to_header_validation_error(EOFValidationError err) noexcept {
  using enum EOFValidationError;
  switch (err) {
  case no_terminating_instruction:
  case stack_underflow:
  case stack_overflow:
  case toplevel_container_truncated: // ?
  case undefined_instruction:
  case invalid_max_stack_height:
    return success;
  default:
    return err;
  }
}

} // namespace
} // namespace fzz

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data_ptr,
                                      size_t data_size) noexcept {
  using namespace fzz;

  // FIXME: Fix evmone API to properly handle inputs above the limit.
  if (data_size > 0xc000)
    return -1;
  const bytes_view data{data_ptr, data_size};

  // Play with error categories.
  {
    const auto err = validate_eof(REV, ContainerKind::runtime, data);

    if (err == EOFValidationError::success)
      return 0;

    const auto cat = get_cat(err);
    if (cat == EOFErrCat::other) {
      std::cerr << err << "\n";
      __builtin_trap();
    }
    return 0;
  }

  const auto vh = validate_header(REV, data);
  const auto v_status = validate_eof(REV, ContainerKind::runtime, data);
  assert(v_status != EOFValidationError::impossible);

  const auto evm1_ok = v_status == EOFValidationError::success;
  switch (v_status) {
  // case EOFValidationError::invalid_non_returning_flag: // incorrect
  // case EOFValidationError::success:                    // incorrect
  // break;
  default: {
    // std::cerr << "XXXX " << v_status << "\n";
    const auto revm_ok = fzz_revm_validate_eof(data_ptr, data_size);
    if (revm_ok != evm1_ok) {
      std::cerr << "evm1: " << v_status << "\n"
                << "revm: " << revm_ok << "\n"
                << "code: " << hex(data) << "\n"
                << "size: " << data.size() << "\n";
      std::abort();
    }
  }
  }

  // FIXME: Besu disabled
  // switch (v_status) {
  // // case EOFValidationError::incompatible_container_kind:
  // // break;
  // default: {
  //   // std::cerr << "XXXX " << v_status << "\n";
  //   const auto ok = fzz_besu_validate_eof(data_ptr, data_size);
  //   if (ok != evm1_ok) {
  //     std::cerr << "evm1: " << v_status << "\n"
  //               << "besu: " << ok << "\n"
  //               << "code: " << hex(data) << "\n"
  //               << "size: " << data.size() << "\n";
  //     std::abort();
  //   }
  // }
  // }

  const auto p_vh_status = std::get_if<EOFValidationError>(&vh);
  assert(p_vh_status == nullptr || *p_vh_status != EOFValidationError::success);
  const auto vh_status =
      (p_vh_status != nullptr) ? *p_vh_status : EOFValidationError::success;
  assert(vh_status != EOFValidationError::impossible);

  if (v_status == EOFValidationError::success &&
      vh_status != EOFValidationError::success)
    __builtin_trap();

  //    if (const auto expected = to_header_validation_error(v_status);
  //    vh_status != expected)
  //    {
  //        std::cerr << "vh_status: " << vh_status << "\nexpected:  " <<
  //        expected
  //                  << "\neof: " << evmc::hex(data) << "\n";
  //        __builtin_trap();
  //    }

  if (v_status == EOFValidationError::success) {
    const auto h = read_valid_eof1_header(data);

    test::eof_bytecode bc{bytes{h.get_code(data, 0)},
                          h.types[0].max_stack_height};
    bc.data(bytes{h.get_data(data)});

    for (size_t i = 1; i < h.code_sizes.size(); ++i)
      bc.code(bytes{h.get_code(data, i)}, h.types[i].inputs, h.types[i].outputs,
              h.types[i].max_stack_height);

    for (size_t i = 0; i < h.container_sizes.size(); ++i)
      bc.container(bytes{h.get_container(data, i)});

    const auto serialized = test::bytecode{bc};
    if (serialized != data) {
      std::cerr << "input: " << hex(data) << "\n";
      std::cerr << "bc:    " << hex(serialized) << "\n";

      __builtin_trap();
    }
  }
  return 0;
}
