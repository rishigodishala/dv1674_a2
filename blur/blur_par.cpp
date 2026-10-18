#include "filters.hpp"
#include "ppm.hpp"
#include "pthread_utils.hpp"
#include <exception>
#include <iostream>

int main(int argc, char const* argv[])
{
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " [radius] [infile] [outfile] [num_threads]\n";
        return 1;
    }
    try {
        const unsigned radius = Threading::parse_positive(argv[1], "radius", Filter::Gauss::max_radius);
        const unsigned threads = Threading::parse_positive(argv[4], "num_threads");
        PPM::Reader reader;
        PPM::Writer writer;
        auto image = reader(argv[2]);
        auto result = Filter::blur_parallel(image, static_cast<int>(radius), threads);
        writer(result, argv[3]);
    } catch (const std::exception& error) {
        std::cerr << "blur_par: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
