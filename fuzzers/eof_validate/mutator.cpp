#include "eof_validate.hpp"
#include "evmone/constants.hpp"
#include <evmone/instructions_opcodes.hpp>

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

  evmone::EOF1Header hdr_;

  static constexpr auto PREFIX_SIZE = 3;
  static constexpr auto SELECTOR_SIZE = 1;
  static constexpr auto NUM_SIZE = 2;

  static inline const auto MINIMAL_EOF =
      evmc::from_spaced_hex("ef0001 010004 0200010001 040000 00 008000 fe")
          .value();

  void patch_types_size(uint16_t x) noexcept {
    const auto p = &data_[PREFIX_SIZE + SELECTOR_SIZE];
    p[0] = x >> 8;
    p[1] = x & 0xff;
  }
  void patch_codes_count(uint16_t x) noexcept {
    const auto p =
        &data_[PREFIX_SIZE + SELECTOR_SIZE + NUM_SIZE + SELECTOR_SIZE];
    p[0] = x >> 8;
    p[1] = x & 0xff;
  }
  uint8_t* get_code_size_ptr(size_t idx) noexcept {
    const auto code_size_off = 3 + 3 + 3 + 2 * idx;
    return data_ + code_size_off;
  }
  void patch_code_size(size_t idx, uint16_t x) noexcept {
    const auto p = get_code_size_ptr(idx);
    p[0] = x >> 8;
    p[1] = x;
  }
  void patch_subcontainers_count(uint16_t x) noexcept {
    const auto p =
        &data_[PREFIX_SIZE + SELECTOR_SIZE + NUM_SIZE + SELECTOR_SIZE +
               NUM_SIZE + NUM_SIZE * hdr_.code_sizes.size() + SELECTOR_SIZE];
    p[0] = x >> 8;
    p[1] = x & 0xff;
  }
  uint8_t* get_subcontainer_size_ptr(size_t idx) noexcept {
    const auto container_size_off =
        3 + 3 + 3 + 2 * hdr_.code_sizes.size() + 3 + 2 * idx;
    return &data_[container_size_off];
  }
  void patch_subcontainer_size(size_t idx, uint16_t x) noexcept {
    const auto p = get_subcontainer_size_ptr(idx);
    p[0] = x >> 8;
    p[1] = x & 0xff;
  }

  size_t mutate_all() noexcept {
    return LLVMFuzzerMutate(data_, size_, max_size_);
  }

  size_t mutate_types() noexcept {
    // Just mutate the type section in-place without changing its size.
    // TODO: Any better ideas?
    const auto types = get_types(hdr_, data_);
    LLVMFuzzerMutate(types.data(), types.size(), types.size());
    return size_;
  }

  size_t mutate_data() noexcept {
    const auto data = hdr_.get_data({data_, size_});
    const auto extra_size = max_size_ - size_;
    const auto max_data_size = data.size() + extra_size;
    if (max_data_size == 0)
      return size_;
    const auto new_data_size =
        // FIXME: LLVMFuzzerMutate works incorrectly with max_size 0. Wrap it.
        LLVMFuzzerMutate(const_cast<uint8_t*>(data.data()), data.size(),
                         max_data_size);
    const auto data_size_off = 3 + 3 + 3 + 2 * hdr_.code_sizes.size() + 3 +
                               2 * hdr_.container_sizes.size() + 3;
    data_[data_size_off] = new_data_size >> 8;
    data_[data_size_off + 1] = new_data_size;

    const auto size_diff = new_data_size - data.size();
    return size_ + size_diff;
  }

  size_t add_code() noexcept {
    // TODO: Maybe better strategy is duplicating existing code?
    // TODO: Drop CALLF/JUMPF to this code section in some other code?
    const auto extra_size = max_size_ - size_;
    if (extra_size < 2 + 4 + 1) {
      // This happens from time to time, but it doesn't seem to be a problem
      // if we return unmodified.
      return size_; // No change. TODO: does 0 indicates anything?
    }

    const auto cnt = hdr_.code_sizes.size();
    patch_types_size((cnt + 1) * 4);
    patch_codes_count(cnt + 1);

    auto data_end = data_ + size_;
    const auto codes_sizes_end =
        &data_[PREFIX_SIZE + SELECTOR_SIZE + NUM_SIZE + SELECTOR_SIZE +
               NUM_SIZE + cnt * NUM_SIZE];
    std::memmove(codes_sizes_end + NUM_SIZE, codes_sizes_end,
                 data_end - codes_sizes_end);
    data_end += NUM_SIZE;
    patch_code_size(cnt, 1);

    const auto new_type = &data_[hdr_.code_offsets.front() + NUM_SIZE];
    std::memmove(new_type + 4, new_type, data_end - new_type);
    data_end += 4;

    const auto t = static_cast<uint8_t>(rand_());
    const auto nonreturning = t >= 0x80;

    if (nonreturning) {
      new_type[0] = 0;
      new_type[1] = 0x80;
      new_type[2] = 0;
      new_type[3] = 0;
    } else {
      new_type[0] = t;
      new_type[1] = t;
      new_type[2] = 0;
      new_type[3] = t;
    }

    const auto new_code = &data_[hdr_.code_offsets.back() +
                                 hdr_.code_sizes.back() + NUM_SIZE + 4];
    std::memmove(new_code + 1, new_code, data_end - new_code);
    data_end += 1;
    new_code[0] = static_cast<uint8_t>(nonreturning ? evmone::OP_INVALID
                                                    : evmone::OP_RETF);

    // TODO: Validate.
    const auto new_size = data_end - data_;
    if (new_size > evmone::MAX_INITCODE_SIZE) {
      std::cerr << "NCODEold:" << size_ << "\nnew:" << new_size << " "
                << MINIMAL_EOF.size();
      std::abort();
    }
    assert(new_size <= evmone::MAX_INITCODE_SIZE);
    return new_size;
  }

  size_t remove_code() noexcept {
    const auto cnt = hdr_.code_sizes.size();
    assert(cnt > 0);
    const auto idx = rand_() % cnt;

    patch_types_size((cnt - 1) * 4);
    patch_codes_count(cnt - 1);

    auto data_end = data_ + size_;
    const auto code_size_p = get_code_size_ptr(idx);
    data_end -= NUM_SIZE;
    std::memmove(code_size_p, code_size_p + NUM_SIZE, data_end - code_size_p);

    const auto types = get_types(hdr_, data_);
    const auto type_ptr = &types[idx * 4] - NUM_SIZE;
    data_end -= 4;
    std::memmove(type_ptr, type_ptr + 4, data_end - type_ptr);

    const auto code = &data_[hdr_.code_offsets[idx]] - NUM_SIZE - 4;
    const auto code_size = hdr_.code_sizes[idx];
    data_end -= code_size;
    std::memmove(code, code + code_size, data_end - code);

    // Validate.
    const auto new_size = static_cast<size_t>(data_end - data_);
    const auto hdr_err = evmone::validate_header(REV, {data_, new_size});
    if (std::holds_alternative<evmone::EOFValidationError>(hdr_err)) {
      const auto err = std::get<evmone::EOFValidationError>(hdr_err);
      if (err == evmone::EOFValidationError::zero_section_size && cnt == 1)
        return size_;
      if (err == evmone::EOFValidationError::invalid_first_section_type &&
          idx == 0)
        return size_;
      std::cerr << err << '\n';
      std::abort();
    }

    assert(std::holds_alternative<evmone::EOF1Header>(hdr_err));
    const auto& new_hdr = std::get<evmone::EOF1Header>(hdr_err);
    assert(new_hdr.code_sizes.size() == hdr_.code_sizes.size() - 1);
    return new_size;
  }

  size_t add_subcontainer() noexcept {
    if (hdr_.container_sizes.empty()) {
      // If not subcontainers, there is no 03 section.
      // Let fuzzer figure out how to add at least one.
      return size_;
    }

    const auto extra_size = max_size_ - size_;
    if (extra_size < MINIMAL_EOF.size() + NUM_SIZE) {
      return size_;
    }

    const auto cnt = hdr_.container_sizes.size();
    patch_subcontainers_count(cnt + 1);

    auto data_end = data_ + size_;
    const auto cont_size_p = get_subcontainer_size_ptr(cnt);
    std::memmove(cont_size_p + NUM_SIZE, cont_size_p, data_end - cont_size_p);
    data_end += NUM_SIZE;
    patch_subcontainer_size(cnt, MINIMAL_EOF.size());

    const auto new_cont = &data_[hdr_.data_offset + NUM_SIZE];
    std::memmove(new_cont + MINIMAL_EOF.size(), new_cont, data_end - new_cont);
    data_end += MINIMAL_EOF.size();
    std::memcpy(new_cont, MINIMAL_EOF.data(), MINIMAL_EOF.size());

    // TODO: Validate.
    const auto new_size = data_end - data_;
    if (new_size > evmone::MAX_INITCODE_SIZE) {
      std::cerr << "old:" << size_ << "\nnew:" << new_size << " "
                << MINIMAL_EOF.size();
      std::abort();
    }
    assert(new_size <= evmone::MAX_INITCODE_SIZE);
    return new_size;
  }

  size_t remove_subcontainer() noexcept {
    const auto cnt = hdr_.container_sizes.size();
    if (cnt == 0)
      return size_;
    const auto idx = rand_() % cnt;

    evmc::bytes old{data_, size_};

    patch_subcontainers_count(cnt - 1);

    auto data_end = data_ + size_;
    const auto cont_size_p = get_subcontainer_size_ptr(idx);
    data_end -= NUM_SIZE;
    std::memmove(cont_size_p, cont_size_p + NUM_SIZE, data_end - cont_size_p);

    const auto cont = &data_[hdr_.container_offsets[idx]] - NUM_SIZE;
    const auto cont_size = hdr_.container_sizes[idx];
    data_end -= cont_size;
    std::memmove(cont, cont + cont_size, data_end - cont);

    // Validate.
    const auto new_size = static_cast<size_t>(data_end - data_);
    const auto hdr_err = evmone::validate_header(REV, {data_, new_size});
    if (std::holds_alternative<evmone::EOFValidationError>(hdr_err)) {
      const auto err = std::get<evmone::EOFValidationError>(hdr_err);
      if (err == evmone::EOFValidationError::zero_section_size && cnt == 1)
        return size_; // TODO: For cnt==1 we want to remove whole 03 section.
      std::cerr << err << ' ' << idx << '\n'
                << evmc::hex({data_, new_size}) << '\n'
                << evmc::hex(old) << '\n';
      std::abort();
    }

    /*
    ef0001 010004 0200010001 0300020001 0400040000800000017e
    ef0001 010004 0200010001 0300020001 0001 0400040000800000017e00

     */

    assert(std::holds_alternative<evmone::EOF1Header>(hdr_err));
    const auto& new_hdr = std::get<evmone::EOF1Header>(hdr_err);
    assert(new_hdr.container_sizes.size() == hdr_.container_sizes.size() - 1);
    return new_size;
  }

  // Mutate code section together with its type.
  // TODO: Alternatives:
  // - mutate type and code section separately.
  // - mutate type in the context of the whole type section.
  size_t mutate_code(size_t code_idx) noexcept {

    // TODO: Remove when stable.
    // const evmc::bytes orig{data_, size_};

    uint8_t scratch[evmone::MAX_INITCODE_SIZE];
    const auto extra_size = max_size_ - size_;
    const auto types = get_types(hdr_, data_);
    const auto type_ptr = &types[code_idx * 4];
    const size_t code_off = hdr_.code_offsets[code_idx];
    const size_t code_size = hdr_.code_sizes[code_idx];

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
    patch_code_size(code_idx, new_code_size);

    // Validate.
    assert(new_size <= evmone::MAX_INITCODE_SIZE);
    assert(new_size <= max_size_);
    const auto header_or_err = evmone::validate_header(REV, {data_, new_size});
    if (std::holds_alternative<evmone::EOFValidationError>(header_or_err)) {
      const auto err = get<evmone::EOFValidationError>(header_or_err);
      if (get_cat(err) == EOFErrCat::header ||
          get_cat(err) == EOFErrCat::body) {
        std::cerr << "code mutation failed: " << err << "\n"
                  << "idx: " << code_idx << "\n"
                  << "new code size: " << new_code_size << "\n"
                  << "new code type: " << evmc::hex({scratch, 4})
                  << "\n"
                  // << evmc::hex(orig) << "\n"
                  << evmc::hex({data_, new_size}) << "\n";
        std::abort();
      }
    }

    return new_size;
  }

  size_t mutate_subcontainer(size_t cont_idx) noexcept {
    const evmone::bytes_view container{data_, size_};
    const auto subcontainer = hdr_.get_container(container, cont_idx);

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

    patch_subcontainer_size(cont_idx, new_subcontainer_size);

    const auto new_size = size_ + size_diff;

    // Validate.
    assert(new_size <= evmone::MAX_INITCODE_SIZE);
    assert(new_size <= max_size_);
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

  size_t inject_instruction() noexcept {
    static constexpr std::array INSTRUCTIONS{evmone::OP_JUMPF,
                                             evmone::OP_CALLF};

    const auto instr = INSTRUCTIONS[rand_() % INSTRUCTIONS.size()];

    switch (instr) {
    case evmone::OP_CALLF:
    case evmone::OP_JUMPF: {
      static constexpr size_t req_size = 3;
      const auto start_idx = rand_() % hdr_.code_sizes.size();
      const auto target_idx = rand_() % hdr_.code_sizes.size();
      const auto code = hdr_.get_code({data_, size_}, start_idx);
      if (code.size() < req_size)
        break;
      const auto pos = rand_() % (code.size() - req_size + 1);
      const auto p = const_cast<uint8_t*>(&code[pos]);
      p[0] = static_cast<uint8_t>(instr);
      p[1] = target_idx >> 8;
      p[2] = target_idx;
      break;
    }
    default:
      __builtin_trap();
    }
    return size_;
  }

public:
  EOFMutator(uint8_t* data, size_t size, size_t max_size, uint32_t seed)
      : data_{data}, size_{size}, max_size_{max_size}, rand_{seed} {}

  size_t mutate() {
    if (size_ > evmone::MAX_INITCODE_SIZE) {
      // TODO: Not sure why it happens.
      return size_;
    }

    evmone::bytes_view container{data_, size_};
    assert(container.size() <= evmone::MAX_INITCODE_SIZE);

    const auto err =
        evmone::validate_eof(REV, evmone::ContainerKind::runtime, container);
    if (err != evmone::EOFValidationError::success) {
      const auto err_cat = get_cat(err);
      if (err_cat == EOFErrCat::header || err_cat == EOFErrCat::body) {
        // If we have invalid header let's keep default fuzzing until it
        // generates something reasonable.
        // TODO: We also do this for "body" for now because out of better ideas.
        return mutate_all();
      }
    }

    assert(container.size() <= evmone::MAX_INITCODE_SIZE);
    assert(container.size() <= max_size_);
    auto header_or_err = evmone::validate_header(REV, container);
    if (std::holds_alternative<evmone::EOFValidationError>(header_or_err)) {
      // TODO(evmone): validate_header also validates types.
      assert(get<evmone::EOFValidationError>(header_or_err) == err);
      assert(get_cat(err) == EOFErrCat::type);
      return mutate_all();
    }

    hdr_ = std::get<evmone::EOF1Header>(std::move(header_or_err));

    static constexpr std::array SINGLETON_MUTATIONS{
        &EOFMutator::mutate_all,         &EOFMutator::mutate_types,
        &EOFMutator::mutate_data,        &EOFMutator::add_code,
        &EOFMutator::remove_code,        &EOFMutator::add_subcontainer,
        &EOFMutator::inject_instruction, &EOFMutator::remove_subcontainer,
    };

    const auto sample_size = SINGLETON_MUTATIONS.size() +
                             hdr_.code_sizes.size() +
                             hdr_.container_sizes.size();
    auto sample = rand_() % sample_size;
    if (sample < SINGLETON_MUTATIONS.size()) {
      const auto new_size = (this->*SINGLETON_MUTATIONS[sample])();
      assert(new_size <= evmone::MAX_INITCODE_SIZE);
      return new_size;
    }
    sample -= SINGLETON_MUTATIONS.size();
    if (sample < hdr_.code_sizes.size())
      return mutate_code(sample);
    sample -= hdr_.code_sizes.size();
    assert(sample < hdr_.container_sizes.size());
    // TODO: Assign higher priority to subcontainers.
    return mutate_subcontainer(sample);
  }
};

} // namespace
} // namespace fzz

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data, size_t size,
                                          size_t max_size, unsigned seed) {
  // TODO: Add custom CrossOver.
  const auto new_size = fzz::EOFMutator{data, size, max_size, seed}.mutate();
  assert(new_size <= evmone::MAX_INITCODE_SIZE);
  return new_size;
  // return fzz::mutate_container(data_ptr, data_size, data_max_size, seed);
}
