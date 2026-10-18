#include "analysis.hpp"
#include "pthread_utils.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace Analysis {
namespace {
// Same element-wise operations and reduction order as the sequential Vector
// methods. Each worker owns a separate output row.
void normalize(const Vector& input, double* output)
{
    const std::size_t length = input.get_size();
    const double mean = input.mean();
    for (std::size_t k = 0; k < length; ++k)
        output[k] = input[static_cast<unsigned>(k)] - mean;
    double squared = 0.0;
    for (std::size_t k = 0; k < length; ++k) squared += output[k] * output[k];
    const double magnitude = std::sqrt(squared);
    for (std::size_t k = 0; k < length; ++k) output[k] /= magnitude;
}
double clipped(double correlation)
{
    return std::max(std::min(correlation, 1.0), -1.0);
}
std::size_t pairs(std::size_t rows)
{
    if (rows < 2) return 0;
    const std::size_t a = rows % 2 == 0 ? rows / 2 : rows;
    const std::size_t b = rows % 2 == 0 ? rows - 1 : (rows - 1) / 2;
    if (a > std::numeric_limits<std::size_t>::max() / b)
        throw std::length_error("too many dataset pairs");
    return a * b;
}
} // namespace

double pearson_parallel(const Vector& first, const Vector& second, unsigned num_threads)
{
    if (!num_threads) throw std::invalid_argument("num_threads must be positive");
    if (first.get_size() != second.get_size() || first.get_size() == 0)
        throw std::invalid_argument("vectors must have the same positive length");
    if (num_threads == 1) return pearson(first, second);
    const std::size_t length = first.get_size();
    std::vector<double> x(length), y(length), products(length);
    normalize(first, x.data());
    normalize(second, y.data());
    Threading::parallel_ranges(length, num_threads, [&](std::size_t begin, std::size_t end) {
        for (std::size_t k = begin; k < end; ++k) products[k] = x[k] * y[k];
    });
    // Ordered summation preserves the original floating-point result. The
    // many-pair path instead parallelizes pairs, avoiding nested thread teams.
    double correlation = 0.0;
    for (std::size_t k = 0; k < length; ++k) correlation += products[k];
    return clipped(correlation);
}

std::vector<double> correlation_coefficients_parallel(
    const std::vector<Vector>& datasets, unsigned num_threads)
{
    if (!num_threads) throw std::invalid_argument("num_threads must be positive");
    const std::size_t rows = datasets.size();
    if (rows < 2) return {};
    const std::size_t length = datasets.front().get_size();
    if (!length) throw std::invalid_argument("vectors must have positive length");
    for (const auto& dataset : datasets)
        if (dataset.get_size() != length)
            throw std::invalid_argument("vectors must have equal lengths");
    if (num_threads == 1) return correlation_coefficients(datasets);
    if (rows == 2) return {pearson_parallel(datasets[0], datasets[1], num_threads)};
    if (length > std::numeric_limits<std::size_t>::max() / rows)
        throw std::length_error("normalized dataset size overflow");

    // Immutable row-major data for the pair phase; normalization is also split
    // across workers. Joining this team completes all rows before pair reads.
    std::vector<double> normalized(rows * length);
    Threading::parallel_ranges(rows, num_threads, [&](std::size_t begin, std::size_t end) {
        for (std::size_t row = begin; row < end; ++row)
            normalize(datasets[row], normalized.data() + row * length);
    });

    const std::size_t count = pairs(rows);
    std::vector<double> result(count);
    Threading::parallel_ranges(count, num_threads, [&](std::size_t begin, std::size_t end) {
        // Prefix(row) counts the pairs before (row,row+1). Binary search maps
        // each range start to its row in the upper-triangular pair matrix.
        auto prefix = [&](std::size_t row) { return count - pairs(rows - row); };
        std::size_t low = 0, high = rows - 1;
        while (low + 1 < high) {
            const std::size_t middle = low + (high - low) / 2;
            if (prefix(middle) <= begin) low = middle;
            else high = middle;
        }
        std::size_t first = low;
        std::size_t second = first + 1 + begin - prefix(first);
        // Equal numbers of pairs balance the triangular workload. Writes go
        // to distinct preallocated slots in the original output ordering.
        for (std::size_t index = begin; index < end; ++index) {
            const double* x = normalized.data() + first * length;
            const double* y = normalized.data() + second * length;
            double correlation = 0.0;
            for (std::size_t k = 0; k < length; ++k) correlation += x[k] * y[k];
            result[index] = clipped(correlation);
            if (++second == rows) { ++first; second = first + 1; }
        }
    });
    return result;
}
} // namespace Analysis
