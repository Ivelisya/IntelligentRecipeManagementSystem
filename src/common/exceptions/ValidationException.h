#ifndef RECIPECLI_VALIDATIONEXCEPTION_H
#define RECIPECLI_VALIDATIONEXCEPTION_H

#include "BaseException.h"
#include <string>

namespace RecipeApp {
namespace Common {
namespace Exceptions {

/**
 * @brief Exception for errors during data validation.
 *
 * Typically thrown when user input or data from a source fails validation checks.
 */
class ValidationException : public RecipeCliBaseException {
public:
    /**
     * @brief Constructs a ValidationException with a given message.
     * @param message The validation error message.
     */
    explicit ValidationException(const std::string& message)
        : RecipeCliBaseException(message) {}

    /**
     * @brief Constructs a ValidationException with a given message (C-style string).
     * @param message The validation error message.
     */
    explicit ValidationException(const char* message)
        : RecipeCliBaseException(message) {}
};

} // namespace Exceptions
} // namespace Common
} // namespace RecipeApp

#endif //RECIPECLI_VALIDATIONEXCEPTION_H