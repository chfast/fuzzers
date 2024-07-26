#pragma once
#include <cstddef>
#include <cstdint>

extern "C" {
bool fzz_besu_validate_eof(const uint8_t*, size_t) noexcept;
}
