#ifndef USER_COMMAND_HANDLER_H
#define USER_COMMAND_HANDLER_H

#include "cxxopts.hpp"
#include "logic/user/UserManager.h"
// Removed PersistenceManager, RecipeManager, RestaurantManager includes as they are no longer needed directly

namespace RecipeApp
{
    namespace CliHandlers
    {
        /**
         * @brief Handles user-related CLI commands.
         */
        class UserCommandHandler
        {
        public:
            /**
             * @brief Constructor.
             * @param userManager Reference to the UserManager.
             */
            explicit UserCommandHandler(UserManager &userManager); // Simplified constructor

            /**
             * @brief Handles the 'login' command.
             * @param result The parsed command line options.
             * @return int Exit code.
             */
            int handleLogin(const cxxopts::ParseResult &result);

            /**
             * @brief Handles the 'register' command.
             * @param result The parsed command line options.
             * @return int Exit code.
             */
            int handleRegister(const cxxopts::ParseResult &result);

            /**
             * @brief Handles the 'logout' command.
             * @param result The parsed command line options.
             * @return int Exit code.
             */
            int handleLogout(const cxxopts::ParseResult &result);

            /**
             * @brief Handles the 'user-profile' command.
             * @param result The parsed command line options.
             * @return int Exit code.
             */
            int handleUserProfile(const cxxopts::ParseResult &result);

            /**
             * @brief Handles the 'update-profile' command.
             * @param result The parsed command line options.
             * @return int Exit code.
             */
            int handleUpdateProfile(const cxxopts::ParseResult &result);

        private:
            UserManager &m_userManager;
            // Removed m_persistenceManager, m_recipeManager, m_restaurantManager
        };

    } // namespace CliHandlers
} // namespace RecipeApp

#endif // USER_COMMAND_HANDLER_H