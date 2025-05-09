#ifndef RECIPE_COMMAND_HANDLER_H
#define RECIPE_COMMAND_HANDLER_H

#include "cxxopts.hpp"
#include "logic/recipe/RecipeManager.h"
// #include "logic/user/UserManager.h" // UserManager dependency removed

namespace RecipeApp
{
    namespace CliHandlers
    {
        class RecipeCommandHandler
        {
        public:
            explicit RecipeCommandHandler(RecipeApp::RecipeManager &recipeManager);

            int handleAddRecipe(const cxxopts::ParseResult &result); // Will be updated to support --tags
            int handleListRecipes(const cxxopts::ParseResult &result);
            int handleViewRecipe(const cxxopts::ParseResult &result);
            int handleSearchRecipes(const cxxopts::ParseResult &result); // Will be updated to support --tag or --tags
            int handleUpdateRecipe(const cxxopts::ParseResult &result);  // Will be updated to support --tags
            int handleDeleteRecipe(const cxxopts::ParseResult &result);

        private:
            RecipeApp::RecipeManager &recipeManager;
            // RecipeApp::UserManager &userManager; // UserManager dependency removed
        };
    } // namespace CliHandlers
} // namespace RecipeApp

#endif // RECIPE_COMMAND_HANDLER_H