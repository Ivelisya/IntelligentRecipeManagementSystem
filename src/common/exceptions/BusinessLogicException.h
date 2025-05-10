#ifndef RECIPECLI_BUSINESSLOGICEXCEPTION_H
#define RECIPECLI_BUSINESSLOGICEXCEPTION_H

#include "BaseException.h"
#include <string>

namespace RecipeApp {
namespace Common {
namespace Exceptions {

/**
 * @brief Exception for errors occurring in the business logic layer.
 *
 * These are errors that are not necessarily due to invalid input or
 * persistence failures, but rather to violations of business rules
 * or unexpected states within the application's core logic.
 */
class BusinessLogicException : public RecipeCliBaseException {
public:
    /**
     * @brief Constructs a BusinessLogicException with a given message.
     * @param message The business logic error message.
     */
    explicit BusinessLogicException(const std::string& message)
        : RecipeCliBaseException(message) {}

    /**
     * @brief Constructs a BusinessLogicException with a given message (C-style string).
     * @param message The business logic error message.
     */
    explicit BusinessLogicException(const char* message)
        : RecipeCliBaseException(message) {}
};

} // namespace Exceptions
} // namespace Common
} // namespace RecipeApp

#endif //RECIPECLI_BUSINESSLOGICEXCEPTION_H