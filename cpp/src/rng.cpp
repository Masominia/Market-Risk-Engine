#include "riskcore/rng.hpp"

#include <cmath>

namespace rc {

static inline std::uint64_t rotl(std::uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }

Rng::Rng(std::uint64_t seed) {
    std::uint64_t z = seed;
    for (auto& v : s_) {
        z += 0x9E3779B97F4A7C15ULL;
        std::uint64_t x = z;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
        v = x ^ (x >> 31);
    }
}

std::uint64_t Rng::next() {
    const std::uint64_t result = rotl(s_[0] + s_[3], 23) + s_[0];
    const std::uint64_t t = s_[1] << 17;
    s_[2] ^= s_[0];
    s_[3] ^= s_[1];
    s_[1] ^= s_[2];
    s_[0] ^= s_[3];
    s_[2] ^= t;
    s_[3] = rotl(s_[3], 45);
    return result;
}

double Rng::uniform() { return (static_cast<double>(next() >> 11) + 0.5) / 9007199254740992.0; }

double Rng::normal() {
    const double u = uniform(), v = uniform();
    return std::sqrt(-2.0 * std::log(u)) * std::cos(6.283185307179586 * v);
}

std::uint64_t Rng::below(std::uint64_t n) {
    const auto k = static_cast<std::uint64_t>(uniform() * static_cast<double>(n));
    return k < n ? k : n - 1;
}

}  // namespace rc
