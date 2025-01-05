// xoshiro256++ generator seeded through splitmix64.
#pragma once

#include <cstdint>

namespace rc {

class Rng {
  public:
    explicit Rng(std::uint64_t seed);
    std::uint64_t next();
    double uniform();  // open interval (0, 1)
    double normal();
    std::uint64_t below(std::uint64_t n);

  private:
    std::uint64_t s_[4];
};

}  // namespace rc
