#ifndef RECIPECLI_CONFIGURATIONEXCEPTION_H
#define RECIPECLI_CONFIGURATIONEXCEPTION_H

#include "BaseException.h"
#include <string>

namespace RecipeApp {
namespace Common {
namespace Exceptions {

/**
 * @brief Exception for errors related to application configuration.
 *
 * This can include issues like missing configuration files, invalid
 * configuration values, or problems encountered while loading or
 * processing configuration data.
 */
class ConfigurationException : public RecipeCliBaseException {
public:
    /**
     * @brief Constructs a ConfigurationException with a given message.
     * @param message The configuration error message.
     */
    explicit ConfigurationException(const std::string& message)
        : RecipeCliBaseException(message) {}

    /**
     * @brief Constructs a ConfigurationException with a given message (C-style string).
     * @param message The configuration error message.
     */
    explicit ConfigurationException(const char* message)
        : RecipeCliBaseException(message) {}
};

} // namespace Exceptions
} // namespace Common
} // namespace RecipeApp

#endif //RECIPECLI_CONFIGURATIONEXCEPTION_H