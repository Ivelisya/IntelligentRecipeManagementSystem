#ifndef RECIPE_COMMAND_HANDLER_H
#define RECIPE_COMMAND_HANDLER_H

#include "cxxopts.hpp"
#include "logic/recipe/RecipeManager.h"
#include "logic/user/UserManager.h"

namespace RecipeApp
{
    namespace CliHandlers
    {
        class RecipeCommandHandler
        {
        public:
            RecipeCommandHandler(RecipeApp::RecipeManager &recipeManager, RecipeApp::UserManager &userManager);

            int handleAddRecipe(const cxxopts::ParseResult &result);
            int handleListRecipes(const cxxopts::ParseResult &result);
            int handleViewRecipe(const cxxopts::ParseResult &result);
            int handleSearchRecipes(const cxxopts::ParseResult &result);
            int handleUpdateRecipe(const cxxopts::ParseResult &result);
            int handleDeleteRecipe(const cxxopts::ParseResult &result);

        private:
            RecipeApp::RecipeManager &recipeManager;
            RecipeApp::UserManager &userManager;
        };
    } // namespace CliHandlers
} // namespace RecipeApp

#endif // RECIPE_COMMAND_HANDLER_H