#ifndef RECIPECLI_PERSISTENCEEXCEPTION_H
#define RECIPECLI_PERSISTENCEEXCEPTION_H

#include "BaseException.h"
#include <string>

namespace RecipeApp {
namespace Common {
namespace Exceptions {

/**
 * @brief Exception for errors during data persistence operations.
 *
 * Examples include file I/O errors, database connection issues, etc.
 */
class PersistenceException : public RecipeCliBaseException {
public:
    /**
     * @brief Constructs a PersistenceException with a given message.
     * @param message The persistence error message.
     */
    explicit PersistenceException(const std::string& message)
        : RecipeCliBaseException(message) {}

    /**
     * @brief Constructs a PersistenceException with a given message (C-style string).
     * @param message The persistence error message.
     */
    explicit PersistenceException(const char* message)
        : RecipeCliBaseException(message) {}
};

} // namespace Exceptions
} // namespace Common
} // namespace RecipeApp

#endif //RECIPECLI_PERSISTENCEEXCEPTION_H