#ifndef ADMIN_COMMAND_HANDLER_H
#define ADMIN_COMMAND_HANDLER_H

#include "cxxopts.hpp"
#include "logic/user/UserManager.h"

namespace RecipeApp
{
    namespace CliHandlers
    {
        class AdminCommandHandler
        {
        public:
            AdminCommandHandler(RecipeApp::UserManager &userManager);

            int handleAdminUserList(const cxxopts::ParseResult &result);
            int handleAdminUserCreate(const cxxopts::ParseResult &result);
            int handleAdminUserUpdate(const cxxopts::ParseResult &result);
            int handleAdminUserDelete(const cxxopts::ParseResult &result);

        private:
            RecipeApp::UserManager &userManager;
        };
    } // namespace CliHandlers
} // namespace RecipeApp

#endif // ADMIN_COMMAND_HANDLER_H