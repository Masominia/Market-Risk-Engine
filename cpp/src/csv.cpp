#include "riskcore/csv.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace rc {

static std::vector<std::string> split(const std::string& line) {
    std::vector<std::string> out;
    std::string cell;
    std::stringstream ss(line);
    while (std::getline(ss, cell, ',')) out.push_back(cell);
    return out;
}

Panel read_prices(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open " + path);
    std::string line;
    if (!std::getline(in, line)) throw std::runtime_error("empty file " + path);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    Panel p;
    auto header = split(line);
    if (header.size() < 2) throw std::runtime_error("no asset columns in " + path);
    p.names.assign(header.begin() + 1, header.end());
    std::vector<double> values;
    std::size_t rows = 0;
    while (std::getline(in, line)) {
        if (line.empty() || line == "\r") continue;
        const auto cells = split(line);
        if (cells.size() != header.size()) throw std::runtime_error("row " + std::to_string(rows + 1) + " has the wrong number of columns");
        for (std::size_t j = 1; j < cells.size(); ++j) {
            const double v = std::stod(cells[j]);
            if (!(v > 0.0)) throw std::runtime_error("non-positive price in row " + std::to_string(rows + 1));
            values.push_back(v);
        }
        ++rows;
    }
    p.prices.rows = rows;
    p.prices.cols = p.names.size();
    p.prices.a = std::move(values);
    return p;
}

}  // namespace rc
