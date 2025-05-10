#include "RecipeEncyclopediaCommandHandler.h"
#include "cli/CliUtils.h" // For printRecipeDetails, printRecipeSummary etc.
#include "cli/ExitCodes.h" // For exit codes
#include "domain/recipe/Recipe.h" // For RecipeApp::Recipe
#include <iostream>
#include <vector>
#include <string>
#include <sstream> // Required for std::ostringstream

namespace RecipeApp
{
    namespace CliHandlers
    {
        RecipeEncyclopediaCommandHandler::RecipeEncyclopediaCommandHandler(RecipeApp::Logic::Encyclopedia::RecipeEncyclopediaManager &manager)
            : encyclopediaManager(manager) {}

        int RecipeEncyclopediaCommandHandler::handleSearchEncyclopediaRecipes(const cxxopts::ParseResult &result)
        {
            if (result.count("enc-search")) // Use the option name defined in main.cpp
            {
                std::string keywords = result["enc-search"].as<std::string>(); // Use the option name
                std::vector<RecipeApp::Recipe> recipes = encyclopediaManager.searchRecipes(keywords);

                if (recipes.empty())
                {
                    std::cout << "No recipes found matching your keywords: '" << keywords << "'." << std::endl;
                }
                else
                {
                    std::cout << "Found " << recipes.size() << " recipes matching '" << keywords << "':" << std::endl;
                    for (const auto &recipe : recipes)
                    {
                        // Print a summary for each recipe
                        // Assuming a utility function like printRecipeSummary exists or we define one
                        // For now, just printing name and ID
                        std::cout << "  ID: " << recipe.getRecipeId() << ", Name: " << recipe.getName() << std::endl;
                        // TODO: Implement a more detailed summary if needed, e.g., using a CliUtils function
                        // CliUtils::printRecipeSummary(recipe); 
                    }
                }
                return RecipeApp::Cli::EX_OK; // Correct constant name
            }
            else
            {
                // This branch should ideally not be hit if main.cpp ensures "enc-search" has a value
                // or cxxopts enforces it. But as a fallback:
                std::cerr << "Error: Missing keyword for encyclopedia search (should be tied to --enc-search option)." << std::endl;
                return RecipeApp::Cli::EX_USAGE; // Correct constant name
            }
        }

        int RecipeEncyclopediaCommandHandler::handleViewEncyclopediaRecipe(const cxxopts::ParseResult &result)
        {
            if (result.count("enc-view")) // Check for the correct option name
            {
                int recipeId = 0;
                try {
                    recipeId = result["enc-view"].as<int>(); // Get value from the correct option
                    if (recipeId <= 0) {
                        std::cerr << "Error: Recipe ID provided via --enc-view must be a positive integer." << std::endl;
                        return RecipeApp::Cli::EX_APP_INVALID_INPUT; // Correct constant name
                    }
                } catch (const cxxopts::exceptions::exception& e) { // Catching a more general cxxopts exception
                    std::cerr << "Error: Invalid Recipe ID format for --enc-view. Please provide an integer. " << e.what() << std::endl;
                    return RecipeApp::Cli::EX_APP_INVALID_INPUT; // Correct constant name
                }

                std::optional<RecipeApp::Recipe> recipeOpt = encyclopediaManager.getRecipeById(recipeId);

                if (recipeOpt)
                {
                    // Assuming a utility function like printRecipeDetails exists
                    // For now, using getDisplayDetails()
                    std::cout << recipeOpt->getDisplayDetails() << std::endl;
                    // CliUtils::printRecipeDetails(*recipeOpt);
                }
                else
                {
                    std::cout << "Recipe with ID " << recipeId << " not found in the encyclopedia." << std::endl;
                }
                return RecipeApp::Cli::EX_OK; // Correct constant name
            }
            else
            {
                // This case should ideally not be reached if the command dispatch in main.cpp ensures --enc-view is present.
                // However, if called directly without the option.
                std::cerr << "Error: --enc-view option is required to view a recipe by ID." << std::endl;
                return RecipeApp::Cli::EX_USAGE; // Correct constant name
            }
        }
    } // namespace CliHandlers
} // namespace RecipeApp