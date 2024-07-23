#include "impl.hpp"
#include <algorithm>
#include <bit>
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
  const auto words_be = reinterpret_cast<const uint64_t*>(bytes_be);
  Scalar out;
  out.data[0] = std::byteswap(words_be[3]);
  out.data[1] = std::byteswap(words_be[2]);
  out.data[2] = std::byteswap(words_be[1]);
  out.data[3] = std::byteswap(words_be[0]);
  return out;
}

// Notation warning: Yellow Paper's p is the same libff's q.
// Returns x < p (YP notation).
static bool valid_element_of_fp(const Scalar& x) noexcept {
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

// TODO: std::expected doesn't work.
static std::pair<libff::alt_bn128_G2, Result>
decode_g2_element(const uint8_t bytes_be[128]) noexcept {
  const auto x = decode_fp2_element(bytes_be);
  if (!x)
    return {{}, Result::invalid_g2};

  const auto y = decode_fp2_element(bytes_be + 64);
  if (!y)
    return {{}, Result::invalid_g2};

  if (x->is_zero() && y->is_zero())
    return {libff::alt_bn128_G2::zero(), Result::one};

  libff::alt_bn128_G2 point{*x, *y, libff::alt_bn128_Fq2::one()};
  if (!point.is_well_formed())
    return {{}, Result::invalid_g2};

  if (!(libff::alt_bn128_G2::order() * point).is_zero()) {
    // wrong order, doesn't belong to the subgroup G2
    return {{}, Result::invalid_g2_subgroup};
  }

  return {point, Result::one};
}

Result libff_pairing_verify(bytes_view input) noexcept {
  if (input.size() % PAIR_SIZE != 0) {
    return Result::invalid_input_length;
  }
  const size_t k = input.size() / PAIR_SIZE;

  init_libff();

  static const auto ONE{libff::alt_bn128_Fq12::one()};
  auto accumulator = ONE;

  for (size_t i{0}; i < k; ++i) {
    auto a{decode_g1_element(&input[i * PAIR_SIZE])};
    if (!a)
      return Result::invalid_g1;

    const auto [b, res] = decode_g2_element(&input[i * PAIR_SIZE + 64]);
    if (res != Result::one)
      return res;

    if (a->is_zero() || b.is_zero()) {
      continue;
    }

    accumulator =
        accumulator * alt_bn128_miller_loop(alt_bn128_precompute_G1(*a),
                                            alt_bn128_precompute_G2(b));
  }

  if (alt_bn128_final_exponentiation(accumulator) == ONE)
    return Result::one;
  return Result::zero;
}

static void encode_fe(uint8_t out[FE_SIZE],
                      const libff::alt_bn128_Fq& f) noexcept {
  auto x = f.as_bigint();
  std::memcpy(&out[0], x.data, FE_SIZE);
  std::reverse(out, out + FE_SIZE);
}

static void encode_g1_element(uint8_t out[G1_SIZE],
                              libff::alt_bn128_G1 p) noexcept {
  std::memset(out, 0, G1_SIZE);
  if (p.is_zero()) {
    return;
  }

  p.to_affine_coordinates();
  encode_fe(out, p.X);
  encode_fe(out + 32, p.Y);
}

static void encode_g2_element(uint8_t out[G2_SIZE],
                              libff::alt_bn128_G2 p) noexcept {
  std::memset(out, 0, G2_SIZE);
  if (p.is_zero()) {
    return;
  }

  p.to_affine_coordinates();
  encode_fe(out, p.X.c1);
  encode_fe(out + FE_SIZE, p.X.c0);
  encode_fe(out + G1_SIZE, p.Y.c1);
  encode_fe(out + G1_SIZE + FE_SIZE, p.Y.c0);
}

void libff_generate_abc(uint8_t data[2 * PAIR_SIZE]) {
  const auto a = libff::alt_bn128_Fr{to_scalar(data)};
  const auto b = libff::alt_bn128_Fr{to_scalar(data + FE_SIZE)};

  const auto c = a * b;
  const auto A = a * libff::alt_bn128_G1::G1_one;
  const auto B = b * libff::alt_bn128_G2::G2_one;
  const auto nC = -c * libff::alt_bn128_G1::G1_one;
  const auto G = libff::alt_bn128_G2::G2_one;

  encode_g1_element(data, A);
  encode_g2_element(data + G1_SIZE, B);
  encode_g1_element(data + PAIR_SIZE, nC);
  encode_g2_element(data + PAIR_SIZE + G1_SIZE, G);
}

void libff_generate_abcd(uint8_t data[2 * PAIR_SIZE]) {
  const auto a = libff::alt_bn128_Fr{to_scalar(data)};
  const auto b = libff::alt_bn128_Fr{to_scalar(data + FE_SIZE)};
  const auto c = libff::alt_bn128_Fr{to_scalar(data + 2 * FE_SIZE)};

  if (c.is_zero()) // Fallback to generate abc
    return libff_generate_abc(data);

  const auto d = a * b * c.inverse();
  assert(a * b == c * d);

  const auto A = a * libff::alt_bn128_G1::G1_one;
  const auto B = b * libff::alt_bn128_G2::G2_one;
  const auto nC = -c * libff::alt_bn128_G1::G1_one;
  const auto D = d * libff::alt_bn128_G2::G2_one;

  encode_g1_element(data, A);
  encode_g2_element(data + G1_SIZE, B);
  encode_g1_element(data + PAIR_SIZE, nC);
  encode_g2_element(data + PAIR_SIZE + G1_SIZE, D);
}

void libff_generate_wrong_g2(uint8_t data[4 * FE_SIZE]) {
  const auto x =
      libff::alt_bn128_Fq2{{to_scalar(&data[0])}, {to_scalar(&data[FE_SIZE])}};
  const auto y2 = x.squared() * x + libff::alt_bn128_twist_coeff_b;
  const auto y = y2.sqrt();
  libff::alt_bn128_G2 p{x, y, libff::alt_bn128_Fq2::one()};

  // The p may not be "well formed" because sqrt may be incorrect.
  // assert(p.is_well_formed())

  // unlikely the generated point will be from G2 subgroup, but it happens.
  // assert(!(libff::alt_bn128_G2::order() * p).is_zero());
  encode_g2_element(data, p);
}

void libff_generate_wrong_g2_pair(uint8_t data[2 * PAIR_SIZE]) {
  const auto s = libff::alt_bn128_Fr{to_scalar(&data[0])};
  const auto x = libff::alt_bn128_Fq2{{to_scalar(&data[FE_SIZE])},
                                      {to_scalar(&data[2 * FE_SIZE])}};

  const auto y2 = x.squared() * x + libff::alt_bn128_twist_coeff_b;
  const auto y = y2.sqrt();
  libff::alt_bn128_G2 p{x, y, libff::alt_bn128_Fq2::one()};

  // The p may not be "well formed" because sqrt may be incorrect.
  // assert(p.is_well_formed())

  // unlikely the generated point will be from G2 subgroup, but it happens.
  // assert(!(libff::alt_bn128_G2::order() * p).is_zero());

  // TODO: Instead of multiplying, load the S point from data.
  const auto S = s * libff::alt_bn128_G1::G1_one;
  const auto nS = -S;
  encode_g1_element(&data[0], S);
  encode_g2_element(&data[G1_SIZE], p);
  encode_g1_element(&data[PAIR_SIZE], nS);
  encode_g2_element(&data[PAIR_SIZE + G1_SIZE], p);
}
