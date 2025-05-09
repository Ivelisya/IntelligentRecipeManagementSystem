#ifndef EXIT_CODES_H
#define EXIT_CODES_H

#include <exception>
#include <stdexcept>

// Standardized exit codes, inspired by sysexits.h

namespace RecipeApp
{
    namespace Cli
    {
        // Generic success
        const int EX_OK = 0; // successful termination

        // Command line usage errors
        const int EX_USAGE = 64;       // command line usage error
        const int EX_DATAERR = 65;     // data format error
        const int EX_NOINPUT = 66;     // cannot open input
        const int EX_NOUSER = 67;      // addressee unknown
        const int EX_NOHOST = 68;      // host name unknown
        const int EX_UNAVAILABLE = 69; // service unavailable
        const int EX_SOFTWARE = 70;    // internal software error
        const int EX_OSERR = 71;       // system error (e.g., can't fork)
        const int EX_OSFILE = 72;      // critical OS file missing
        const int EX_CANTCREAT = 73;   // can't create (user) output file
        const int EX_IOERR = 74;       // input/output error
        const int EX_TEMPFAIL = 75;    // temp failure; user is invited to retry
        const int EX_PROTOCOL = 76;    // remote error in protocol
        const int EX_NOPERM = 77;      // permission denied
        const int EX_CONFIG = 78;      // configuration error

        // Application-specific errors (can start from a custom base, e.g., 80)
        const int EX_APP_LOGIN_FAILED = 80;
        const int EX_APP_NOT_LOGGED_IN = 81;
        const int EX_APP_PERMISSION_DENIED = 82; // More specific than EX_NOPERM for app logic
        const int EX_APP_ITEM_NOT_FOUND = 83;
        const int EX_APP_INVALID_INPUT = 84;    // For app-level validation beyond basic data format
        const int EX_APP_OPERATION_FAILED = 85; // Generic application operation failure
        const int EX_APP_ALREADY_EXISTS = 86;   // Item already exists (e.g. username, recipe name)

        // Custom Exception Classes
        class RecipeAppException : public std::runtime_error
        {
        public:
            RecipeAppException(const std::string &message) : std::runtime_error(message) {}
        };

        class RecipeNotFoundException : public RecipeAppException
        {
        public:
            RecipeNotFoundException(const std::string &message) : RecipeAppException(message) {}
        };

        class InvalidRecipeDataException : public RecipeAppException
        {
        public:
            InvalidRecipeDataException(const std::string &message) : RecipeAppException(message) {}
        };

        class PersistenceException : public RecipeAppException
        {
        public:
            PersistenceException(const std::string &message) : RecipeAppException(message) {}
        };

        class AuthorizationException : public RecipeAppException
        {
        public:
            AuthorizationException(const std::string &message) : RecipeAppException(message) {}
        };
    } // namespace Cli
} // namespace RecipeApp

#endif // EXIT_CODES_H