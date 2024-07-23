#include <common/hex.hpp>
#include "impl.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

std::string_view to_string(Result r) noexcept {
  switch (r) {
  case Result::zero:
    return "negative";
  case Result::one:
    return "positive";
  case Result::invalid_input_length:
    return "invalid_input_length";
  case Result::invalid_g1:
    return "invalid_g1_point";
  case Result::invalid_g2:
    return "invalid_g2_point";
  case Result::invalid_g2_subgroup:
    return "invalid_g2_subgroup";
  default:
    return "<unknown>";
  }
}

int main(int argc, const char* argv[]) {
  if (argc != 3)
    std::cerr << "usage:\n" << argv[0] << " corpus_dir output_file\n";

  std::map<int, std::vector<fzz::bytes>> collection;

  for (auto const& dir_entry : std::filesystem::directory_iterator{argv[1]}) {
    std::ifstream in_file{dir_entry.path()};
    fzz::bytes data(std::istreambuf_iterator<char>{in_file},
                    std::istreambuf_iterator<char>{});

    if (data.size() > 1152)
      continue;

    const auto r = libff_pairing_verify(data);
    collection[static_cast<int>(r)].emplace_back(std::move(data));
  }

  std::ofstream out{argv[2]};
  out << "    data:\n";
  for (auto& [r, inputs] : collection) {
    std::ranges::stable_sort(inputs, [](const auto& a, const auto& b) {
      return a.size() == b.size() ? a < b : a.size() < b.size();
    });
    for (const auto& input : inputs) {
      out << "      - :label " << to_string(Result(r)) << " :raw 0x"
          << fzz::hex(input) << "\n";
    }
  }
}
