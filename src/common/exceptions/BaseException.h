#ifndef RECIPECLI_BASEEXCEPTION_H
#define RECIPECLI_BASEEXCEPTION_H

#include <stdexcept>
#include <string>

namespace RecipeApp {
namespace Common {
namespace Exceptions {

/**
 * @brief Base class for all custom exceptions in the Recipe CLI application.
 *
 * Inherits from std::runtime_error to provide a standard what() method.
 */
class RecipeCliBaseException : public std::runtime_error {
public:
    /**
     * @brief Constructs a RecipeCliBaseException with a given message.
     * @param message The error message.
     */
    explicit RecipeCliBaseException(const std::string& message)
        : std::runtime_error(message) {}

    /**
     * @brief Constructs a RecipeCliBaseException with a given message (C-style string).
     * @param message The error message.
     */
    explicit RecipeCliBaseException(const char* message)
        : std::runtime_error(message) {}
};

} // namespace Exceptions
} // namespace Common
} // namespace RecipeApp

#endif //RECIPECLI_BASEEXCEPTION_H