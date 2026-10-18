#include "analysis.hpp"
#include "dataset.hpp"
#include "pthread_utils.hpp"
#include <exception>
#include <iostream>

int main(int argc, char const* argv[])
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " [infile] [outfile] [num_threads]\n";
        return 1;
    }
    try {
        const unsigned threads = Threading::parse_positive(argv[3], "num_threads");
        auto datasets = Dataset::read(argv[1]);
        if (datasets.empty()) throw std::invalid_argument("no datasets were read");
        auto result = Analysis::correlation_coefficients_parallel(datasets, threads);
        Dataset::write(result, argv[2]);
    } catch (const std::exception& error) {
        std::cerr << "pearson_par: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
