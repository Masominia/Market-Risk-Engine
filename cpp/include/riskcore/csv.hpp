// Reader for the price panel: a header line, then one row per day; the first column is a day counter.
#pragma once

#include <string>
#include <vector>

#include "riskcore/matrix.hpp"

namespace rc {

struct Panel {
    std::vector<std::string> names;
    Matrix prices;  // days x assets
};

Panel read_prices(const std::string& path);

}  // namespace rc
