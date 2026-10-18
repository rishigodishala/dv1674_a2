/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "filters.hpp"
#include "matrix.hpp"
#include "ppm.hpp"
#include <cmath>

namespace Filter
{

    namespace Gauss
    {
        void get_weights(int n, double *weights_out)
        {
            for (auto i{0}; i <= n; i++)
            {
                double x{static_cast<double>(i) * max_x / n};
                weights_out[i] = exp(-x * x * pi);
            }
        }
    }

    Matrix blur(Matrix m, const int radius)
    {
        Matrix scratch{PPM::max_dimension};
        auto dst{m};

        // Resolve planar buffers once instead of calling accessors per sample.
        const auto dst_stride = dst.get_x_size();
        const auto scratch_stride = scratch.get_x_size();
        auto* dst_r = &dst.r(0, 0);
        auto* dst_g = &dst.g(0, 0);
        auto* dst_b = &dst.b(0, 0);
        auto* scratch_r = &scratch.r(0, 0);
        auto* scratch_g = &scratch.g(0, 0);
        auto* scratch_b = &scratch.b(0, 0);

        // The radius is fixed: compute weights once for both passes.
        double w[Gauss::max_radius + 1]{};
        Gauss::get_weights(radius, w);

        for (auto x{0}; x < dst.get_x_size(); x++)
        {
            for (auto y{0}; y < dst.get_y_size(); y++)
            {
                // unsigned char Matrix::r(unsigned x, unsigned y) const
                // {
                //     return R[y * x_size + x];
                // }

                auto r{w[0] * dst_r[(y) * dst_stride + (x)]}, g{w[0] * dst_g[(y) * dst_stride + (x)]}, b{w[0] * dst_b[(y) * dst_stride + (x)]}, n{w[0]};

                for (auto wi{1}; wi <= radius; wi++)
                {
                    auto wc{w[wi]};
                    auto x2{x - wi};
                    if (x2 >= 0)
                    {
                        r += wc * dst_r[(y) * dst_stride + (x2)];
                        g += wc * dst_g[(y) * dst_stride + (x2)];
                        b += wc * dst_b[(y) * dst_stride + (x2)];
                        n += wc;
                    }
                    x2 = x + wi;
                    if (x2 < dst.get_x_size())
                    {
                        r += wc * dst_r[(y) * dst_stride + (x2)];
                        g += wc * dst_g[(y) * dst_stride + (x2)];
                        b += wc * dst_b[(y) * dst_stride + (x2)];
                        n += wc;
                    }
                }
                scratch_r[(y) * scratch_stride + (x)] = r / n;
                scratch_g[(y) * scratch_stride + (x)] = g / n;
                scratch_b[(y) * scratch_stride + (x)] = b / n;
            }
        }

        for (auto x{0}; x < dst.get_x_size(); x++)
        {
            for (auto y{0}; y < dst.get_y_size(); y++)
            {
                auto r{w[0] * scratch_r[(y) * scratch_stride + (x)]}, g{w[0] * scratch_g[(y) * scratch_stride + (x)]}, b{w[0] * scratch_b[(y) * scratch_stride + (x)]}, n{w[0]};

                for (auto wi{1}; wi <= radius; wi++)
                {
                    auto wc{w[wi]};
                    auto y2{y - wi};
                    if (y2 >= 0)
                    {
                        r += wc * scratch_r[(y2) * scratch_stride + (x)];
                        g += wc * scratch_g[(y2) * scratch_stride + (x)];
                        b += wc * scratch_b[(y2) * scratch_stride + (x)];
                        n += wc;
                    }
                    y2 = y + wi;
                    if (y2 < dst.get_y_size())
                    {
                        r += wc * scratch_r[(y2) * scratch_stride + (x)];
                        g += wc * scratch_g[(y2) * scratch_stride + (x)];
                        b += wc * scratch_b[(y2) * scratch_stride + (x)];
                        n += wc;
                    }
                }
                dst_r[(y) * dst_stride + (x)] = r / n;
                dst_g[(y) * dst_stride + (x)] = g / n;
                dst_b[(y) * dst_stride + (x)] = b / n;
            }
        }

        return dst;
    }

}
