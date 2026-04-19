#ifndef TRACER_HPP
#define TRACER_HPP

#include <chrono>
#include <string>
#include "Logger.hpp"

#define TRACE(component, span) ScopedTrace _trace_##__LINE__(component, span)

class ScopedTrace {
public:
    ScopedTrace(const std::string& component, const std::string& span)
        : component_(component), span_(span),
          start_(std::chrono::high_resolution_clock::now()) {}

    ~ScopedTrace() {
        auto end = std::chrono::high_resolution_clock::now();
        auto us  = std::chrono::duration_cast<std::chrono::microseconds>(
                       end - start_).count();
        LOG_DEBUG(component_, "span_end", "span=" + span_ + " duration_us=" + std::to_string(us));
    }
private:
    std::string component_, span_;
    std::chrono::high_resolution_clock::time_point start_;
};

#endif
