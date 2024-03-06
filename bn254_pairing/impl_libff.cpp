#include "impl.hpp"
#include <algorithm>
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

  bool b_in_g2 = true;

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

    if (!(libff::alt_bn128_G2::order() * *b).is_zero()) {
      // wrong order, doesn't belong to the subgroup G2
      b_in_g2 = false;
    }

    accumulator =
        accumulator * alt_bn128_miller_loop(alt_bn128_precompute_G1(*a),
                                            alt_bn128_precompute_G2(*b));
  }

  if (alt_bn128_final_exponentiation(accumulator) == ONE) {
    if (!b_in_g2)
      return Result::invalid_g2_subgroup;
    return Result::one;
  }
  if (!b_in_g2)
    return Result::invalid_g2_subgroup;
  return Result::zero;
}

static void encode_fe(uint8_t out[32], const libff::alt_bn128_Fq &f) noexcept {
  auto x = f.as_bigint();
  std::memcpy(&out[0], x.data, 32);
  std::reverse(out, out + 32);
}

static void encode_g1_element(uint8_t out[64], libff::alt_bn128_G1 p) noexcept {
  std::memset(out, 0, 64);
  if (p.is_zero()) {
    return;
  }

  p.to_affine_coordinates();
  encode_fe(out, p.X);
  encode_fe(out + 32, p.Y);
}

static void encode_g2_element(uint8_t out[128],
                              libff::alt_bn128_G2 p) noexcept {
  std::memset(out, 0, 128);
  if (p.is_zero()) {
    return;
  }

  p.to_affine_coordinates();
  encode_fe(out, p.X.c1);
  encode_fe(out + 32, p.X.c0);
  encode_fe(out + 64, p.Y.c1);
  encode_fe(out + 96, p.Y.c0);
}

void libff_generate_abc(uint8_t data[2 * STRIDE_SIZE]) {

  mpz_t x0, x1;
  mpz_inits(x0, x1, nullptr);
  mpz_import(x0, 32, 1, 1, 0, 0, data);
  mpz_import(x1, 32, 1, 1, 0, 0, data + FE_SIZE);
  const auto a = libff::alt_bn128_Fr{x0};
  const auto b = libff::alt_bn128_Fr{x1};
  mpz_clears(x0, x1, nullptr);

  const auto c = a * b;
  const auto A = a * libff::alt_bn128_G1::G1_one;
  const auto B = b * libff::alt_bn128_G2::G2_one;
  const auto nC = -c * libff::alt_bn128_G1::G1_one;
  const auto G = libff::alt_bn128_G2::G2_one;

  encode_g1_element(data, A);
  encode_g2_element(data + 64, B);
  encode_g1_element(data + 192, nC);
  encode_g2_element(data + 192 + 64, G);

  // const auto r = libff_pairing_verify({out, 2 * STRIDE_SIZE});
  // if (r != Result::one) {
  //   std::cerr << "result: " << int(r) << "\n";
  //   __builtin_trap();
  // }
}

bool libff_generate_abcd(uint8_t data[2 * STRIDE_SIZE]) {

  mpz_t x0, x1, x2;
  mpz_inits(x0, x1, x2, nullptr);
  mpz_import(x0, 32, 1, 1, 0, 0, data);
  mpz_import(x1, 32, 1, 1, 0, 0, data + FE_SIZE);
  mpz_import(x2, 32, 1, 1, 0, 0, data + 2 * FE_SIZE);
  const auto a = libff::alt_bn128_Fr{x0};
  const auto b = libff::alt_bn128_Fr{x1};
  const auto c = libff::alt_bn128_Fr{x2};
  mpz_clears(x0, x1, x2, nullptr);

  if (c.is_zero())
    return false;

  const auto d = a * b * c.inverse();
  assert(a * b == c * d);

  const auto A = a * libff::alt_bn128_G1::G1_one;
  const auto B = b * libff::alt_bn128_G2::G2_one;
  const auto nC = -c * libff::alt_bn128_G1::G1_one;
  const auto D = d * libff::alt_bn128_G2::G2_one;

  encode_g1_element(data, A);
  encode_g2_element(data + 64, B);
  encode_g1_element(data + 192, nC);
  encode_g2_element(data + 192 + 64, D);

  // const auto r = libff_pairing_verify({out, 2 * STRIDE_SIZE});
  // if (r != Result::one) {
  //   std::cerr << "result: " << int(r) << "\n";
  //   __builtin_trap();
  // }
  return true;
}

bool libff_generate_wrong_g2(uint8_t data[4 * FE_SIZE]) {
  mpz_t x0, x1;
  mpz_inits(x0, x1, nullptr);
  mpz_import(x0, 32, 1, 1, 0, 0, &data[0]);
  mpz_import(x1, 32, 1, 1, 0, 0, &data[32]);

  const auto x = libff::alt_bn128_Fq2{{x0}, {x1}};
  mpz_clears(x0, x1, nullptr);
  const auto y2 = x.squared() * x + libff::alt_bn128_twist_coeff_b;
  const auto y = y2.sqrt();
  libff::alt_bn128_G2 p{x, y, libff::alt_bn128_Fq2::one()};
  if (!p.is_well_formed())
    return false;

  // unlikely the generated point will be from G2 subgroup, but it happens.
  // assert(!(libff::alt_bn128_G2::order() * p).is_zero());
  encode_g2_element(data, p);
  return true;
}

bool libff_generate_wrong_g2_pair(uint8_t data[2 * STRIDE_SIZE]) {
  mpz_t x0, x1, x2;
  mpz_inits(x0, x1, x2, nullptr);
  mpz_import(x0, 32, 1, 1, 0, 0, &data[0]);
  mpz_import(x1, 32, 1, 1, 0, 0, &data[64]);
  mpz_import(x2, 32, 1, 1, 0, 0, &data[96]);

  const auto s = libff::alt_bn128_Fr{x0};
  const auto x = libff::alt_bn128_Fq2{{x1}, {x2}};
  mpz_clears(x0, x1, x2, nullptr);

  const auto y2 = x.squared() * x + libff::alt_bn128_twist_coeff_b;
  const auto y = y2.sqrt();
  libff::alt_bn128_G2 p{x, y, libff::alt_bn128_Fq2::one()};
  if (!p.is_well_formed())
    return false;

  // unlikely the generated point will be from G2 subgroup, but it happens.
  // assert(!(libff::alt_bn128_G2::order() * p).is_zero());

  // TODO: Instead of multiplying, load the S point from data.
  const auto S = s * libff::alt_bn128_G1::G1_one;
  const auto nS = -S;
  encode_g1_element(&data[0], S);
  encode_g2_element(&data[64], p);
  encode_g1_element(&data[192], nS);
  encode_g2_element(&data[192 + 64], p);

  // This will validate to true if subgroup check is missing.
  // const auto r = libff_pairing_verify({data, 2 * STRIDE_SIZE});
  // if (r != Result::one) {
  //   std::cerr << "result: " << int(r) << "\n";
  //   __builtin_trap();
  // }
  return true;
}
