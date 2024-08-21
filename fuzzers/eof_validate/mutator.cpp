#include "eof_validate.hpp"
#include "evmone/constants.hpp"

#include <evmc/hex.hpp>
#include <intx/intx.hpp>
#include <iostream>
#include <random>
#include <span>

extern "C" size_t LLVMFuzzerMutate(uint8_t* data, size_t size,
                                   size_t max_size) noexcept;

namespace fzz {
namespace {

std::span<uint8_t> get_types(const evmone::EOF1Header& header,
                             uint8_t* data) noexcept {
  const auto size = header.code_sizes.size() * 4;
  const auto end_pos = header.code_offsets[0];
  const auto begin_pos = end_pos - size;
  return {data + begin_pos, size};
}

class EOFMutator {
  uint8_t* data_;
  size_t size_;
  size_t max_size_;
  std::minstd_rand rand_;

  size_t mutate_types(const evmone::EOF1Header& header) {
    // Just mutate the type section in-place without changing its size.
    // TODO: Any better ideas?
    const auto types = get_types(header, data_);
    LLVMFuzzerMutate(types.data(), types.size(), types.size());
    return size_;
  }

  // Mutate code section together with its type.
  // TODO: Alternatives:
  // - mutate type and code section separately.
  // - mutate type in the context of the whole type section.
  size_t mutate_code(size_t code_idx, const evmone::EOF1Header& header) {

    // TODO: Remove when stable.
    const evmc::bytes orig{data_, size_};

    uint8_t scratch[evmone::MAX_INITCODE_SIZE];
    const auto extra_size = max_size_ - size_;
    const auto types = get_types(header, data_);
    const auto type_ptr = &types[code_idx * 4];
    const size_t code_off = header.code_offsets[code_idx];
    const size_t code_size = header.code_sizes[code_idx];

    std::memcpy(scratch, type_ptr, 4);
    std::memcpy(scratch + 4, data_ + code_off, code_size);
    const size_t type_code_size = code_size + 4;

    const auto new_type_code_size =
        LLVMFuzzerMutate(scratch, type_code_size, type_code_size + extra_size);
    std::memcpy(type_ptr, scratch, 4);

    // Keep the code size at least 1.
    // TODO: Having 0 will make the header invalid.
    const auto new_code_size = std::max(new_type_code_size, size_t{5}) - 4;
    if (new_code_size > 0xffff) {
      std::cerr << code_idx << " " << code_off << " " << code_size << " "
                << extra_size << " " << new_code_size << "\n";
    }
    assert(new_code_size <= 0xffff);
    const size_t after_pos = code_off + code_size;
    const auto after_size = size_ - after_pos;
    const auto new_after_pos = after_pos + (new_code_size - code_size);
    std::memmove(data_ + new_after_pos, data_ + after_pos, after_size);
    std::memcpy(data_ + code_off, scratch + 4, new_code_size);
    const auto new_size = size_ + (new_code_size - code_size);

    // Patch size in header.
    const auto code_size_off = 3 + 3 + 3 + 2 * code_idx;
    data_[code_size_off] = new_code_size >> 8;
    data_[code_size_off + 1] = new_code_size;

    // Validate.
    const auto header_or_err = evmone::validate_header(REV, {data_, new_size});
    if (std::holds_alternative<evmone::EOFValidationError>(header_or_err)) {
      const auto err = get<evmone::EOFValidationError>(header_or_err);
      if (get_cat(err) == EOFErrCat::header ||
          get_cat(err) == EOFErrCat::body) {
        std::cerr << "code mutation failed: " << err << "\n"
                  << "idx: " << code_idx << "\n"
                  << "new code size: " << new_code_size << "\n"
                  << "new code type: " << evmc::hex({scratch, 4}) << "\n"
                  << evmc::hex(orig) << "\n"
                  << evmc::hex({data_, new_size}) << "\n";
        std::abort();
      }
    }

    return new_size;
  }

  size_t mutate_subcontainer(size_t cont_idx,
                             const evmone::EOF1Header& header) {
    const evmone::bytes_view container{data_, size_};
    const auto subcontainer = header.get_container(container, cont_idx);

    uint8_t scratch[evmone::MAX_INITCODE_SIZE];
    const auto extra_size = max_size_ - size_;
    std::memcpy(scratch, subcontainer.data(), subcontainer.size());

    const auto new_subcontainer_size =
        EOFMutator{scratch, subcontainer.size(),
                   subcontainer.size() + extra_size,
                   static_cast<uint32_t>(rand_())}
            .mutate();
    const auto size_diff = new_subcontainer_size - subcontainer.size();
    const auto after = subcontainer.data() + subcontainer.size();
    const auto new_after = after + size_diff;
    const auto after_size = size_ - (after - data_);
    std::memmove(const_cast<uint8_t*>(new_after), after, after_size);

    std::memcpy(const_cast<uint8_t*>(subcontainer.data()), scratch,
                new_subcontainer_size);

    const auto container_size_off =
        3 + 3 + 3 + 2 * header.code_sizes.size() + 3 + 2 * cont_idx;
    data_[container_size_off] = new_subcontainer_size >> 8;
    data_[container_size_off + 1] = new_subcontainer_size;

    const auto new_size = size_ + size_diff;

    // Validate.
    const auto header_or_err = evmone::validate_header(REV, {data_, new_size});
    if (std::holds_alternative<evmone::EOFValidationError>(header_or_err)) {
      const auto err = get<evmone::EOFValidationError>(header_or_err);
      if (get_cat(err) == EOFErrCat::header ||
          get_cat(err) == EOFErrCat::body) {
        std::cerr << "subcontainer mutation failed: " << err << "\n"
                  << evmc::hex({data_, new_size}) << "\n";
        std::abort();
      }
    }

    return new_size;
  }

public:
  EOFMutator(uint8_t* data, size_t size, size_t max_size, uint32_t seed)
      : data_{data}, size_{size}, max_size_{max_size}, rand_{seed} {}

  size_t mutate() {
    evmone::bytes_view container{data_, size_};

    const auto err =
        evmone::validate_eof(REV, evmone::ContainerKind::runtime, container);
    if (err != evmone::EOFValidationError::success) {
      const auto err_cat = get_cat(err);
      if (err_cat == EOFErrCat::header || err_cat == EOFErrCat::body) {
        // If we have invalid header let's keep default fuzzing until it
        // generates something reasonable.
        // TODO: We also do this for "body" for now because out of better ideas.
        return LLVMFuzzerMutate(data_, size_, max_size_);
      }
    }

    const auto header_or_err = evmone::validate_header(REV, container);
    if (std::holds_alternative<evmone::EOFValidationError>(header_or_err)) {
      // TODO(evmone): validate_header also validates types.
      assert(get<evmone::EOFValidationError>(header_or_err) == err);
      assert(get_cat(err) == EOFErrCat::type);
      return LLVMFuzzerMutate(data_, size_, max_size_);
    }

    const auto& header = std::get<evmone::EOF1Header>(header_or_err);

    const auto total_count = header.code_sizes.size() +
                             header.container_sizes.size() +
                             (header.data_size != 0) + 1;

    const auto elem_idx = rand_() % total_count;

    if (elem_idx == total_count - 1) // special index, mutate whole container
      return LLVMFuzzerMutate(data_, size_, max_size_);

    if (elem_idx == 0)
      return mutate_types(header);

    const auto code_idx = elem_idx - 1;
    if (code_idx < header.code_sizes.size())
      return mutate_code(code_idx, header);

    const auto cont_idx = code_idx - header.code_sizes.size();
    if (cont_idx < header.container_sizes.size())
      return mutate_subcontainer(cont_idx, header);

    assert(elem_idx == total_count - 2);

    // TODO:
    return LLVMFuzzerMutate(data_, size_, max_size_);
  }
};

size_t mutate_part(const uint8_t* data, size_t data_size, size_t data_max_size,
                   uint8_t* part, size_t part_size) {
  const auto part_end = part + part_size;
  const auto size_available = data_max_size - data_size;
  const auto after_size = static_cast<size_t>((data + data_size - part_end));
  std::memmove(part_end + size_available, part_end, after_size);
  const auto part_new_size =
      LLVMFuzzerMutate(part, part_size, part_size + size_available);
  const auto part_new_end = part + part_new_size;
  std::memmove(part_new_end, part_end + size_available, after_size);
  const auto size_diff = part_new_size - part_size;
  return data_size + size_diff;
}

size_t mutate_container(uint8_t* data_ptr, size_t data_size,
                        size_t data_max_size, unsigned int seed) {
  const evmone::bytes_view data{data_ptr, data_size};

  const auto vh = evmone::validate_header(REV, data);
  if (std::holds_alternative<evmone::EOFValidationError>(vh))
    return LLVMFuzzerMutate(data_ptr, data_size, data_max_size);

  const auto& header = std::get<evmone::EOF1Header>(vh);
  const auto c_codes = header.code_sizes.size();
  const auto c_subcontainers = header.container_sizes.size();
  const auto c_all = c_codes + c_subcontainers + 2;
  const auto idx = seed % c_all;

  if (idx == 0) // types
  {
    assert(!header.code_offsets.empty());
    const auto types_end = &data_ptr[header.code_offsets.front()];
    const auto types_size = c_codes * 4;
    const auto types_begin = types_end - types_size;
    return mutate_part(data_ptr, data_size, data_max_size, types_begin,
                       types_size);
  } else if (idx == c_all - 1) // data
  {
    const auto d_begin = &data_ptr[header.data_offset];
    const auto d_size = static_cast<size_t>((data_ptr + data_size - d_begin));
    return mutate_part(data_ptr, data_size, data_max_size, d_begin, d_size);
  } else if (idx <= c_codes) {
    const auto code_idx = idx - 1;
    const auto code_begin = &data_ptr[header.code_offsets[code_idx]];
    const auto code_size = header.code_sizes[code_idx];
    return mutate_part(data_ptr, data_size, data_max_size, code_begin,
                       code_size);
  } else {
    const auto cont_idx = idx - 1 - c_codes;
    const auto cont_begin = &data_ptr[header.container_offsets[cont_idx]];
    const auto cont_size = header.container_sizes[cont_idx];
    const auto partEnd = cont_begin + cont_size;
    const auto sizeAvailable = data_max_size - data_size;
    const auto afterSize =
        static_cast<size_t>((data_ptr + data_size - partEnd));
    std::memmove(partEnd + sizeAvailable, partEnd, afterSize);

    const auto seed2 = static_cast<unsigned int>(std::minstd_rand{seed}());
    const auto partNewSize = mutate_container(cont_begin, cont_size,
                                              cont_size + sizeAvailable, seed2);

    const auto partNewEnd = cont_begin + partNewSize;
    std::memmove(partNewEnd, partEnd + sizeAvailable, afterSize);
    const auto sizeDiff = partNewSize - cont_size;
    return data_size + sizeDiff;
  }
}

} // namespace
} // namespace fzz

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data, size_t size,
                                          size_t max_size, unsigned seed) {

  return fzz::EOFMutator{data, size, max_size, seed}.mutate();
  // return fzz::mutate_container(data_ptr, data_size, data_max_size, seed);
}
