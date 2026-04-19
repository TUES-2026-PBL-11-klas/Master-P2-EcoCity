#ifndef PERSISTENCE_EXCEPTION_H
#define PERSISTENCE_EXCEPTION_H

#include <stdexcept>
#include <string>

// Thrown when a MongoDB save or load operation fails.
// Wraps the underlying error message so callers (e.g. GameService) can
// react or log it without catching raw mongocxx exceptions everywhere.
class PersistenceException : public std::runtime_error {
public:
    enum class Operation { SAVE, LOAD };

    PersistenceException(Operation op, const std::string& detail)
        : std::runtime_error(opLabel(op) + " failed: " + detail)
        , operation_(op)
    {}

    Operation getOperation() const { return operation_; }

private:
    Operation operation_;

    static std::string opLabel(Operation op) {
        return op == Operation::SAVE ? "Game save" : "Game load";
    }
};

#endif // PERSISTENCE_EXCEPTION_H
