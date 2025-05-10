#ifndef RECIPE_ENCYCLOPEDIA_COMMAND_HANDLER_H
#define RECIPE_ENCYCLOPEDIA_COMMAND_HANDLER_H

#include "cxxopts.hpp"
#include "logic/encyclopedia/RecipeEncyclopediaManager.h" // Adjusted path

namespace RecipeApp
{
    namespace CliHandlers
    {
        class RecipeEncyclopediaCommandHandler
        {
        public:
            explicit RecipeEncyclopediaCommandHandler(RecipeApp::Logic::Encyclopedia::RecipeEncyclopediaManager &encyclopediaManager);

            // Handles searching recipes in the encyclopedia
            // Expected command: recipe-cli encyclopedia search --keywords "keyword1 keyword2"
            int handleSearchEncyclopediaRecipes(const cxxopts::ParseResult &result);

            // Handles viewing a specific recipe from the encyclopedia by ID
            // Expected command: recipe-cli encyclopedia view --id <recipe_id>
            int handleViewEncyclopediaRecipe(const cxxopts::ParseResult &result);

        private:
            RecipeApp::Logic::Encyclopedia::RecipeEncyclopediaManager &encyclopediaManager;
        };
    } // namespace CliHandlers
} // namespace RecipeApp

#endif // RECIPE_ENCYCLOPEDIA_COMMAND_HANDLER_H