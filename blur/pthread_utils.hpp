#ifndef DV1674_PTHREAD_UTILS_HPP
#define DV1674_PTHREAD_UTILS_HPP
#include <algorithm>
#include <charconv>
#include <cstddef>
#include <exception>
#include <limits>
#include <pthread.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace Threading {
inline unsigned parse_positive(const char* text, const char* name,
                               unsigned maximum = std::numeric_limits<unsigned>::max())
{
    const std::string value{text};
    unsigned number = 0;
    auto result = std::from_chars(value.data(), value.data() + value.size(), number);
    if (result.ec != std::errc{} || result.ptr != value.data() + value.size()
        || number == 0 || number > maximum)
        throw std::invalid_argument(std::string(name) + " must be a positive integer within its supported range");
    return number;
}

// Equal contiguous ranges; the caller participates, so requested counts all
// workers. The function owns [begin,end) and must share inputs read-only.
template <class Function>
void parallel_ranges(std::size_t count, unsigned requested, Function function)
{
    if (!requested) throw std::invalid_argument("num_threads must be positive");
    if (!count) return;
    const std::size_t workers = std::min(count, static_cast<std::size_t>(requested));
    if (workers == 1) { function(0, count); return; }
    struct Task {
        Function* function;
        std::size_t begin, end;
        std::exception_ptr error;
    };
    std::vector<Task> tasks(workers);
    std::vector<pthread_t> handles(workers - 1);
    const std::size_t quotient = count / workers, remainder = count % workers;
    for (std::size_t i = 0; i < workers; ++i) {
        const std::size_t begin = i * quotient + std::min(i, remainder);
        tasks[i] = Task{&function, begin, begin + quotient + (i < remainder), {}};
    }
    auto entry = [](void* argument) -> void* {
        auto& task = *static_cast<Task*>(argument);
        try { (*task.function)(task.begin, task.end); }
        catch (...) { task.error = std::current_exception(); }
        return nullptr;
    };
    std::size_t created = 0;
    int creation_error = 0;
    for (std::size_t i = 1; i < workers; ++i) {
        const int status = pthread_create(&handles[created], nullptr, entry, &tasks[i]);
        if (status) { creation_error = status; break; }
        ++created;
    }
    if (!creation_error) entry(&tasks[0]);
    // Join all started workers even if creating the remaining team failed.
    int join_error = 0;
    for (std::size_t i = 0; i < created; ++i) {
        const int status = pthread_join(handles[i], nullptr);
        if (status && !join_error) join_error = status;
    }
    if (creation_error)
        throw std::system_error(creation_error, std::generic_category(), "pthread_create");
    if (join_error)
        throw std::system_error(join_error, std::generic_category(), "pthread_join");
    for (const auto& task : tasks)
        if (task.error) std::rethrow_exception(task.error);
}
} // namespace Threading
#endif
