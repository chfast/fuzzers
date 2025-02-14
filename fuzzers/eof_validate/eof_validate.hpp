#pragma once
#include <evmone/eof.hpp>

namespace fzz {
constexpr auto REV = EVMC_OSAKA;

enum class EOFErrCat { header, body, type, code, subcont, ref, other };

inline EOFErrCat get_cat(evmone::EOFValidationError err) noexcept {
  using enum evmone::EOFValidationError;
  switch (err) {
  case invalid_prefix:
  case eof_version_unknown:
  case header_terminator_missing:
  case type_section_missing:
  case code_section_missing:
  case data_section_missing:
  case section_headers_not_terminated:
  case zero_section_size:
  case incomplete_section_size:
  case incomplete_section_number:
  case too_many_code_sections:
  case too_many_container_sections:
  case invalid_type_section_size:
    return EOFErrCat::header;
  case invalid_section_bodies_size:
    return EOFErrCat::body;
  case invalid_first_section_type:
  case inputs_outputs_num_above_limit:
  case max_stack_height_above_limit:
  case toplevel_container_truncated: // ?
    return EOFErrCat::type;
  case undefined_instruction:
  case truncated_instruction:
  case invalid_container_section_index:
  case stack_underflow: // stack?
  case unreachable_instructions:
  case invalid_code_section_index:
  case invalid_rjump_destination:
  case callf_to_non_returning_function:
  case invalid_dataloadn_index:
  case no_terminating_instruction:
  case invalid_non_returning_flag:         // ?
  case invalid_max_stack_height:           // stack?
  case stack_height_mismatch:              // stack?
  case stack_overflow:                     // stack?
  case stack_higher_than_outputs_required: // stack?
  case incompatible_container_kind:
  case jumpf_destination_incompatible_outputs:
    return EOFErrCat::code;
  case unreferenced_subcontainer:
  case eofcreate_with_truncated_container:
  case ambiguous_container_kind:
    return EOFErrCat::subcont;
  case unreachable_code_sections:
    return EOFErrCat::ref;
  case container_size_above_limit:
    return EOFErrCat::other;
  case success:
    assert(!"success");
    __builtin_trap();
  case impossible:
    assert(!"impossible");
    __builtin_trap();
  }
}

} // namespace fzz
