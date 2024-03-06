#include "../utils/hex.hpp"
#include "impl.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

int main(int argc, const char *argv[]) {
  if (argc != 3)
    std::cerr << "usage:\n" << argv[0] << " corpus_dir output_file\n";

  std::map<int, std::vector<fzz::bytes>> collection;

  for (auto const &dir_entry : std::filesystem::directory_iterator{argv[1]}) {
    std::ifstream in_file{dir_entry.path()};
    fzz::bytes data(std::istreambuf_iterator<char>{in_file},
                    std::istreambuf_iterator<char>{});

    const auto r = libff_pairing_verify(data);
    collection[static_cast<int>(r)].emplace_back(std::move(data));
  }

  std::ofstream out{argv[2]};
  for (auto &[r, inputs] : collection) {
    out << "// " << r << "\n";
    std::ranges::stable_sort(inputs, [](const auto &a, const auto &b) {
      return a.size() == b.size() ? a < b : a.size() < b.size();
    });
    for (const auto &input : inputs) {
      out << fzz::hex(input) << "\n";
    }
  }
}
