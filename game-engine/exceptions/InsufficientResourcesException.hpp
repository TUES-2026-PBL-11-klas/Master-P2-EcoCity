#ifndef INSUFFICIENT_RESOURCES_EXCEPTION_H
#define INSUFFICIENT_RESOURCES_EXCEPTION_H

#include <stdexcept>
#include <string>

class InsufficientResourcesException : public std::runtime_error {
public:
    InsufficientResourcesException(long long required, long long available)
        : std::runtime_error(
            "Cannot build: requires " + std::to_string(required) +
            " money but only "        + std::to_string(available) + " available"
          )
        , required_(required)
        , available_(available)
    {}

    long long getRequired()  const { return required_;  }
    long long getAvailable() const { return available_; }

private:
    long long required_;
    long long available_;
};

#endif