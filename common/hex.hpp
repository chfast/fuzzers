#include <cstdint>
#include <string>
#include <string_view>

namespace fzz {
/// String of uint8_t chars.
using bytes = std::basic_string<uint8_t>;

/// String view of uint8_t chars.
using bytes_view = std::basic_string_view<uint8_t>;

/// Encode a byte to a hex string.
inline std::string hex(uint8_t b) noexcept {
  static constexpr auto hex_digits = "0123456789abcdef";
  return {hex_digits[b >> 4], hex_digits[b & 0xf]};
}

/// Encodes bytes as hex string.
inline std::string hex(bytes_view bs) {
  std::string str;
  str.reserve(bs.size() * 2);
  for (const auto b : bs)
    str += hex(b);
  return str;
}
} // namespace fzz
