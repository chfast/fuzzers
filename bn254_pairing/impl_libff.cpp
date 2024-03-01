#include "impl.hpp"
#include <libff/algebra/curves/alt_bn128/alt_bn128_pairing.hpp>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <optional>

using Scalar = libff::bigint<libff::alt_bn128_q_limbs>;

// TODO: Initializing with a global initialization doesn't work.
static void init_libff() noexcept {
  // magic static
  [[maybe_unused]] static bool initialized = []() noexcept {
    libff::inhibit_profiling_info = true;
    libff::inhibit_profiling_counters = true;
    libff::alt_bn128_pp::init_public_params();
    return true;
  }();
}

static Scalar to_scalar(const uint8_t bytes_be[32]) noexcept {
  mpz_t m;
  mpz_init(m);
  mpz_import(m, 32, /*order=*/1, /*size=*/1, /*endian=*/0, /*nails=*/0,
             bytes_be);
  Scalar out{m};
  mpz_clear(m);
  return out;
}

// Notation warning: Yellow Paper's p is the same libff's q.
// Returns x < p (YP notation).
static bool valid_element_of_fp(const Scalar &x) noexcept {
  return mpn_cmp(x.data, libff::alt_bn128_modulus_q.data,
                 libff::alt_bn128_q_limbs) < 0;
}

static std::optional<libff::alt_bn128_G1>
decode_g1_element(const uint8_t bytes_be[64]) noexcept {
  Scalar x{to_scalar(bytes_be)};
  if (!valid_element_of_fp(x)) {
    return {};
  }

  Scalar y{to_scalar(bytes_be + 32)};
  if (!valid_element_of_fp(y)) {
    return {};
  }

  if (x.is_zero() && y.is_zero()) {
    return libff::alt_bn128_G1::zero();
  }

  libff::alt_bn128_G1 point{x, y, libff::alt_bn128_Fq::one()};
  if (!point.is_well_formed()) {
    return {};
  }
  return point;
}

static std::optional<libff::alt_bn128_Fq2>
decode_fp2_element(const uint8_t bytes_be[64]) noexcept {
  // big-endian encoding
  Scalar c0{to_scalar(bytes_be + 32)};
  Scalar c1{to_scalar(bytes_be)};

  if (!valid_element_of_fp(c0) || !valid_element_of_fp(c1)) {
    return {};
  }

  return libff::alt_bn128_Fq2{c0, c1};
}

static std::optional<libff::alt_bn128_G2>
decode_g2_element(const uint8_t bytes_be[128]) noexcept {
  std::optional<libff::alt_bn128_Fq2> x{decode_fp2_element(bytes_be)};
  if (!x) {
    return {};
  }

  std::optional<libff::alt_bn128_Fq2> y{decode_fp2_element(bytes_be + 64)};
  if (!y) {
    return {};
  }

  if (x->is_zero() && y->is_zero()) {
    return libff::alt_bn128_G2::zero();
  }

  libff::alt_bn128_G2 point{*x, *y, libff::alt_bn128_Fq2::one()};
  if (!point.is_well_formed()) {
    return {};
  }

  if (!(libff::alt_bn128_G2::order() * point).is_zero()) {
    // wrong order, doesn't belong to the subgroup G2
    return {};
  }

  return point;
}

Result libff_pairing_verify(bytes_view input) noexcept {
  if (input.size() % STRIDE_SIZE != 0) {
    return Result::invalid_input_length;
  }
  const size_t k = input.size() / STRIDE_SIZE;

  init_libff();

  static const auto ONE{libff::alt_bn128_Fq12::one()};
  auto accumulator = ONE;

  for (size_t i{0}; i < k; ++i) {
    auto a{decode_g1_element(&input[i * STRIDE_SIZE])};
    if (!a) {
      return Result::invalid_g1;
    }
    auto b{decode_g2_element(&input[i * STRIDE_SIZE + 64])};
    if (!b) {
      return Result::invalid_g2;
    }

    if (a->is_zero() || b->is_zero()) {
      continue;
    }

    accumulator =
        accumulator * alt_bn128_miller_loop(alt_bn128_precompute_G1(*a),
                                            alt_bn128_precompute_G2(*b));
  }

  if (alt_bn128_final_exponentiation(accumulator) == ONE) {
    return Result::one;
  }
  return Result::zero;
}
