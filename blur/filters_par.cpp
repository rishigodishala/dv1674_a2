#include "filters.hpp"
#include "ppm.hpp"
#include "pthread_utils.hpp"
#include <stdexcept>

namespace Filter {
Matrix blur_parallel(const Matrix& source, int radius, unsigned num_threads)
{
    if (!num_threads) throw std::invalid_argument("num_threads must be positive");
    if (radius < 1 || radius > static_cast<int>(Gauss::max_radius))
        throw std::invalid_argument("radius must be between 1 and 1000");
    const int width = static_cast<int>(source.get_x_size());
    const int height = static_cast<int>(source.get_y_size());
    if (!width || !height) throw std::invalid_argument("empty image");
    if (num_threads == 1 || height == 1) return blur(source, radius);

    // Match the sequential input/destination copies and scratch layout.
    Matrix input{source};
    Matrix scratch{PPM::max_dimension};
    Matrix destination{input};
    const auto dst_stride = destination.get_x_size();
    const auto scratch_stride = scratch.get_x_size();
    auto* dst_r = &destination.r(0, 0);
    auto* dst_g = &destination.g(0, 0);
    auto* dst_b = &destination.b(0, 0);
    auto* scratch_r = &scratch.r(0, 0);
    auto* scratch_g = &scratch.g(0, 0);
    auto* scratch_b = &scratch.b(0, 0);
    double weights[Gauss::max_radius + 1]{};
    Gauss::get_weights(radius, weights);

    // Horizontal pass: read destination, write disjoint scratch rows.
    // Preserve the per-pixel accumulation order and intermediate rounding.
    Threading::parallel_ranges(height, num_threads, [&](std::size_t begin, std::size_t end) {
        for (int x = 0; x < width; ++x) {
            for (int y = static_cast<int>(begin); y < static_cast<int>(end); ++y) {
                const auto center = y * dst_stride + x;
                double r = weights[0] * dst_r[center];
                double g = weights[0] * dst_g[center];
                double b = weights[0] * dst_b[center];
                double norm = weights[0];
                for (int offset = 1; offset <= radius; ++offset) {
                    const double weight = weights[offset];
                    int neighbor = x - offset;
                    if (neighbor >= 0) {
                        const auto index = y * dst_stride + neighbor;
                        r += weight * dst_r[index];
                        g += weight * dst_g[index];
                        b += weight * dst_b[index];
                        norm += weight;
                    }
                    neighbor = x + offset;
                    if (neighbor < width) {
                        const auto index = y * dst_stride + neighbor;
                        r += weight * dst_r[index];
                        g += weight * dst_g[index];
                        b += weight * dst_b[index];
                        norm += weight;
                    }
                }
                const auto output = y * scratch_stride + x;
                scratch_r[output] = r / norm;
                scratch_g[output] = g / norm;
                scratch_b[output] = b / norm;
            }
        }
    });
    // All first-pass workers are joined before the second pass can read scratch.
    Threading::parallel_ranges(height, num_threads, [&](std::size_t begin, std::size_t end) {
        for (int x = 0; x < width; ++x) {
            for (int y = static_cast<int>(begin); y < static_cast<int>(end); ++y) {
                const auto center = y * scratch_stride + x;
                double r = weights[0] * scratch_r[center];
                double g = weights[0] * scratch_g[center];
                double b = weights[0] * scratch_b[center];
                double norm = weights[0];
                for (int offset = 1; offset <= radius; ++offset) {
                    const double weight = weights[offset];
                    int neighbor = y - offset;
                    if (neighbor >= 0) {
                        const auto index = neighbor * scratch_stride + x;
                        r += weight * scratch_r[index];
                        g += weight * scratch_g[index];
                        b += weight * scratch_b[index];
                        norm += weight;
                    }
                    neighbor = y + offset;
                    if (neighbor < height) {
                        const auto index = neighbor * scratch_stride + x;
                        r += weight * scratch_r[index];
                        g += weight * scratch_g[index];
                        b += weight * scratch_b[index];
                        norm += weight;
                    }
                }
                const auto output = y * dst_stride + x;
                dst_r[output] = r / norm;
                dst_g[output] = g / norm;
                dst_b[output] = b / norm;
            }
        }
    });
    return destination;
}
} // namespace Filter
