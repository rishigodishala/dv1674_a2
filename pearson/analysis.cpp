/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "analysis.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <vector>

namespace Analysis {

std::vector<double> correlation_coefficients(const std::vector<Vector>& datasets)
{
    std::vector<double> result {};

    // Each dataset has the same normalization in every pair: prepare it once.
    std::vector<Vector> normalized;
    normalized.reserve(datasets.size());
    for (const auto& dataset : datasets) {
        auto centered { dataset - dataset.mean() };
        normalized.push_back(centered / centered.magnitude());
    }

    for (auto sample1 { 0 }; sample1 + 1 < datasets.size(); sample1++) {
        for (auto sample2 { sample1 + 1 }; sample2 < datasets.size(); sample2++) {
            auto corr { normalized[sample1].dot(normalized[sample2]) };
            result.push_back(std::max(std::min(corr, 1.0), -1.0));
        }
    }

    return result;
}

double pearson(const Vector& vec1, const Vector& vec2)
{
    auto x_mean { vec1.mean() };
    auto y_mean { vec2.mean() };

    auto x_mm { vec1 - x_mean };
    auto y_mm { vec2 - y_mean };

    auto x_mag { x_mm.magnitude() };
    auto y_mag { y_mm.magnitude() };

    auto x_mm_over_x_mag { x_mm / x_mag };
    auto y_mm_over_y_mag { y_mm / y_mag };

    auto r { x_mm_over_x_mag.dot(y_mm_over_y_mag) };

    return std::max(std::min(r, 1.0), -1.0);
}
};
