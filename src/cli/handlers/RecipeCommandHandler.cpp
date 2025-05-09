#include "cli/handlers/RecipeCommandHandler.h"
#include "cli/CliUtils.h"
#include "cli/ExitCodes.h"         // For standardized exit codes
#include "domain/recipe/Recipe.h"  // For RecipeApp::Difficulty
#include "domain/user/User.h"      // For RecipeApp::UserRole
#include "core/CustomLinkedList.h" // For CustomLinkedList type
#include <iostream>
#include <string>
#include <vector>
#include <limits>    // Required for std::numeric_limits
#include <stdexcept> // Required for std::exception

namespace RecipeApp
{
    namespace CliHandlers
    {

        RecipeCommandHandler::RecipeCommandHandler(RecipeApp::RecipeManager &rm, RecipeApp::UserManager &um)
            : recipeManager(rm), userManager(um) {}

        int RecipeCommandHandler::handleAddRecipe(const cxxopts::ParseResult &result)
        {
            const RecipeApp::User *currentUser = userManager.getCurrentUser();
            if (!currentUser)
            {
                std::cerr << "Error: Please login first to add a recipe." << std::endl;
                return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            }

            std::cout << "--- Add New Recipe ---" << std::endl;
            std::string name = RecipeApp::CliUtils::getStringFromConsole("Enter recipe name: ");
            if (name.empty())
            {
                std::cerr << "Error: Recipe name cannot be empty." << std::endl;
                return RecipeApp::Cli::EX_APP_INVALID_INPUT;
            }

            CustomDataStructures::CustomLinkedList<RecipeApp::Recipe> existingRecipes = recipeManager.findRecipeByName(name, false);
            if (!existingRecipes.isEmpty())
            {
                std::cerr << "Error: Recipe name '" << name << "' already exists. Please use a different name." << std::endl;
                return RecipeApp::Cli::EX_APP_ALREADY_EXISTS;
            }

            std::vector<std::pair<std::string, std::string>> ingredients = RecipeApp::CliUtils::getIngredientsFromConsole();
            std::vector<std::string> steps = RecipeApp::CliUtils::getStepsFromConsole();
            int cookingTime = RecipeApp::CliUtils::getIntFromConsole("Enter cooking time (minutes, positive integer): ");
            while (cookingTime <= 0)
            {
                std::cerr << "Error: Cooking time must be a positive integer." << std::endl;
                cookingTime = RecipeApp::CliUtils::getIntFromConsole("Please re-enter cooking time (minutes, positive integer): ");
            }
            RecipeApp::Difficulty difficulty = RecipeApp::CliUtils::getDifficultyFromConsole();
            std::string cuisine = RecipeApp::CliUtils::getStringFromConsole("Enter cuisine: ");
            if (cuisine.empty())
            {
                std::cerr << "Error: Cuisine cannot be empty." << std::endl;
                return RecipeApp::Cli::EX_APP_INVALID_INPUT;
            }
            std::string nutritionalInfo = RecipeApp::CliUtils::getStringFromConsole("Enter nutritional information (optional, can be empty): ");
            std::string imageUrl = RecipeApp::CliUtils::getStringFromConsole("Enter image URL (optional, can be empty): ");

            RecipeApp::Recipe newRecipe(0, name, ingredients, steps, cookingTime, difficulty, cuisine);
            if (!nutritionalInfo.empty())
                newRecipe.setNutritionalInfo(nutritionalInfo);
            if (!imageUrl.empty())
                newRecipe.setImageUrl(imageUrl);

            int newRecipeId = recipeManager.addRecipe(newRecipe);

            if (newRecipeId != -1)
            {
                std::cout << "Recipe '" << name << "' added successfully! (New ID: " << newRecipeId << ")" << std::endl;
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "Failed to add recipe. Name might conflict or an internal error occurred." << std::endl;
                return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
            }
        }

        int RecipeCommandHandler::handleListRecipes(const cxxopts::ParseResult &result)
        {
            std::cout << "--- Recipe List ---" << std::endl;
            const auto &allRecipes = recipeManager.getAllRecipes();
            if (allRecipes.empty())
            {
                std::cout << "No recipes available currently." << std::endl;
                return RecipeApp::Cli::EX_OK; // Not an error, just no data
            }
            for (const auto &recipe : allRecipes)
            {
                RecipeApp::CliUtils::displayRecipeDetailsBrief(recipe);
            }
            std::cout << "Total " << allRecipes.size() << " recipes." << std::endl;
            return RecipeApp::Cli::EX_OK;
        }

        int RecipeCommandHandler::handleViewRecipe(const cxxopts::ParseResult &result)
        {
            if (!result.count("recipe-view"))
            {
                std::cerr << "Error: Missing argument (RECIPE_ID) for recipe-view command." << std::endl;
                std::cerr << "Usage: recipe-cli --recipe-view <RECIPE_ID>" << std::endl;
                return RecipeApp::Cli::EX_USAGE;
            }

            int recipeId = 0;
            try
            {
                recipeId = result["recipe-view"].as<int>();
            }
            catch (const cxxopts::exceptions::exception &e)
            {
                std::cerr << "Error: Invalid RECIPE_ID '" << result["recipe-view"].as<std::string>() << "'. Please enter a number." << std::endl;
                return RecipeApp::Cli::EX_DATAERR;
            }

            std::optional<RecipeApp::Recipe> recipeOpt = recipeManager.findRecipeById(recipeId);
            if (recipeOpt) // Or recipeOpt.has_value()
            {
                RecipeApp::CliUtils::displayRecipeDetailsFull(recipeOpt.value());
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "Recipe with ID " << recipeId << " not found." << std::endl;
                return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
            }
        }

        int RecipeCommandHandler::handleSearchRecipes(const cxxopts::ParseResult &result)
        {
            if (!result.count("recipe-search"))
            {
                std::cerr << "Error: Missing argument (QUERY) for recipe-search command." << std::endl;
                std::cerr << "Usage: recipe-cli --recipe-search <QUERY>" << std::endl;
                return RecipeApp::Cli::EX_USAGE;
            }
            std::string query = result["recipe-search"].as<std::string>();
            if (query.empty())
            {
                std::cerr << "Error: Search query cannot be empty." << std::endl;
                return RecipeApp::Cli::EX_APP_INVALID_INPUT;
            }
            std::cout << "--- Recipe Search Results (Name contains: \"" << query << "\") ---" << std::endl;

            CustomDataStructures::CustomLinkedList<RecipeApp::Recipe> searchResults = recipeManager.findRecipeByName(query, true);

            if (searchResults.isEmpty())
            {
                std::cout << "No recipes found with name containing \"" << query << "\"." << std::endl;
            }
            else
            {
                for (const auto &recipe : searchResults)
                {
                    RecipeApp::CliUtils::displayRecipeDetailsBrief(recipe);
                }
                std::cout << "Found " << searchResults.getSize() << " matching recipes." << std::endl;
            }
            return RecipeApp::Cli::EX_OK; // Search itself is successful even if no results
        }

        int RecipeCommandHandler::handleUpdateRecipe(const cxxopts::ParseResult &result)
        {
            const RecipeApp::User *currentUser = userManager.getCurrentUser();
            if (!currentUser)
            {
                std::cerr << "Error: Please login first to update a recipe." << std::endl;
                return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            }

            if (currentUser->getRole() != RecipeApp::UserRole::Admin)
            {
                std::cerr << "Error: Permission denied. Only administrators can update recipes." << std::endl;
                return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            }

            if (!result.count("recipe-update"))
            {
                std::cerr << "Error: Missing argument (RECIPE_ID) for recipe-update command." << std::endl;
                std::cerr << "Usage: recipe-cli --recipe-update <RECIPE_ID>" << std::endl;
                return RecipeApp::Cli::EX_USAGE;
            }

            int recipeId = 0;
            try
            {
                recipeId = result["recipe-update"].as<int>();
            }
            catch (const cxxopts::exceptions::exception &e)
            {
                std::cerr << "Error: Invalid RECIPE_ID '" << result["recipe-update"].as<std::string>() << "'. Please enter a number." << std::endl;
                return RecipeApp::Cli::EX_DATAERR;
            }

            std::optional<RecipeApp::Recipe> recipeToUpdateOpt = recipeManager.findRecipeById(recipeId);
            if (!recipeToUpdateOpt)
            {
                std::cerr << "Error: Recipe with ID " << recipeId << " not found." << std::endl;
                return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
            }
            RecipeApp::Recipe recipeToUpdate = recipeToUpdateOpt.value(); // Work with a mutable copy

            std::cout << "--- Update Recipe (ID: " << recipeId << ") ---" << std::endl;
            std::cout << "Current recipe information:" << std::endl;
            RecipeApp::CliUtils::displayRecipeDetailsFull(recipeToUpdate);
            std::cout << "Enter new recipe information (leave empty to keep current value for a field):" << std::endl;

            std::string newName = RecipeApp::CliUtils::getStringFromConsole("New name [" + recipeToUpdate.getName() + "]: ");
            if (!newName.empty())
            {
                if (newName != recipeToUpdate.getName())
                {
                    CustomDataStructures::CustomLinkedList<RecipeApp::Recipe> existing = recipeManager.findRecipeByName(newName, false);
                    bool conflict = false;
                    for (const auto &r : existing)
                    {
                        if (r.getRecipeId() != recipeId)
                        {
                            conflict = true;
                            break;
                        }
                    }
                    if (conflict)
                    {
                        std::cerr << "Error: New recipe name '" << newName << "' is already used by another recipe." << std::endl;
                        return RecipeApp::Cli::EX_APP_ALREADY_EXISTS;
                    }
                }
                recipeToUpdate.setName(newName);
            }

            std::cout << "Modify ingredients? (y/n, currently " << recipeToUpdate.getIngredients().size() << " items): ";
            std::string changeIngredients = RecipeApp::CliUtils::getStringFromConsole("");
            if (changeIngredients == "y" || changeIngredients == "Y")
            {
                recipeToUpdate.setIngredients(RecipeApp::CliUtils::getIngredientsFromConsole());
            }

            std::cout << "Modify steps? (y/n, currently " << recipeToUpdate.getSteps().size() << " items): ";
            std::string changeSteps = RecipeApp::CliUtils::getStringFromConsole("");
            if (changeSteps == "y" || changeSteps == "Y")
            {
                recipeToUpdate.setSteps(RecipeApp::CliUtils::getStepsFromConsole());
            }

            std::string newTimeStr = RecipeApp::CliUtils::getStringFromConsole("New cooking time (minutes) [" + std::to_string(recipeToUpdate.getCookingTime()) + "]: ");
            if (!newTimeStr.empty())
            {
                try
                {
                    int newTime = std::stoi(newTimeStr);
                    if (newTime > 0)
                        recipeToUpdate.setCookingTime(newTime);
                    else
                        std::cout << "Invalid cooking time entered. Value will remain unchanged." << std::endl;
                }
                catch (const std::exception &e)
                {
                    std::cout << "Entered cooking time is not a valid number. Value will remain unchanged." << std::endl;
                }
            }

            std::cout << "Modify difficulty? (y/n): ";
            std::string changeDifficulty = RecipeApp::CliUtils::getStringFromConsole("");
            if (changeDifficulty == "y" || changeDifficulty == "Y")
            {
                recipeToUpdate.setDifficulty(RecipeApp::CliUtils::getDifficultyFromConsole());
            }

            std::string newCuisine = RecipeApp::CliUtils::getStringFromConsole("New cuisine [" + recipeToUpdate.getCuisine() + "]: ");
            if (!newCuisine.empty())
                recipeToUpdate.setCuisine(newCuisine);

            std::string newNutritionalInfo = RecipeApp::CliUtils::getStringFromConsole("New nutritional info [" + recipeToUpdate.getNutritionalInfo().value_or("") + "]: ");
            if (!newNutritionalInfo.empty())
                recipeToUpdate.setNutritionalInfo(newNutritionalInfo);
            else
            {
                std::string clearInfo = RecipeApp::CliUtils::getStringFromConsole("Clear nutritional info? (y/n): ");
                if (clearInfo == "y" || clearInfo == "Y")
                    recipeToUpdate.setNutritionalInfo({});
            }

            std::string newImageUrl = RecipeApp::CliUtils::getStringFromConsole("New image URL [" + recipeToUpdate.getImageUrl().value_or("") + "]: ");
            if (!newImageUrl.empty())
                recipeToUpdate.setImageUrl(newImageUrl);
            else
            {
                std::string clearUrl = RecipeApp::CliUtils::getStringFromConsole("Clear image URL? (y/n): ");
                if (clearUrl == "y" || clearUrl == "Y")
                    recipeToUpdate.setImageUrl({});
            }

            if (recipeManager.updateRecipe(recipeToUpdate))
            {
                std::cout << "Recipe ID " << recipeId << " updated successfully!" << std::endl;
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "Failed to update recipe." << std::endl;
                return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
            }
        }

        int RecipeCommandHandler::handleDeleteRecipe(const cxxopts::ParseResult &result)
        {
            const RecipeApp::User *currentUser = userManager.getCurrentUser();
            if (!currentUser)
            {
                std::cerr << "Error: Please login first to delete a recipe." << std::endl;
                return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            }

            if (currentUser->getRole() != RecipeApp::UserRole::Admin)
            {
                std::cerr << "Error: Permission denied. Only administrators can delete recipes." << std::endl;
                return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            }

            if (!result.count("recipe-delete"))
            {
                std::cerr << "Error: Missing argument (RECIPE_ID) for recipe-delete command." << std::endl;
                std::cerr << "Usage: recipe-cli --recipe-delete <RECIPE_ID>" << std::endl;
                return RecipeApp::Cli::EX_USAGE;
            }

            int recipeId = 0;
            try
            {
                recipeId = result["recipe-delete"].as<int>();
            }
            catch (const cxxopts::exceptions::exception &e)
            {
                std::cerr << "Error: Invalid RECIPE_ID '" << result["recipe-delete"].as<std::string>() << "'. Please enter a number." << std::endl;
                return RecipeApp::Cli::EX_DATAERR;
            }

            std::optional<RecipeApp::Recipe> recipeToDeleteOpt = recipeManager.findRecipeById(recipeId);
            if (!recipeToDeleteOpt)
            {
                std::cerr << "Error: Recipe with ID " << recipeId << " not found." << std::endl;
                return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
            }

            std::cout << "Found recipe: " << recipeToDeleteOpt.value().getName() << " (ID: " << recipeId << ")" << std::endl;
            std::string confirm = RecipeApp::CliUtils::getStringFromConsole("Are you sure you want to delete this recipe? (y/n): ");

            if (confirm == "y" || confirm == "Y")
            {
                if (recipeManager.deleteRecipe(recipeId))
                {
                    std::cout << "Recipe ID " << recipeId << " deleted successfully!" << std::endl;
                    return RecipeApp::Cli::EX_OK;
                }
                else
                {
                    std::cerr << "Failed to delete recipe." << std::endl;
                    return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
                }
            }
            else
            {
                std::cout << "Delete operation cancelled." << std::endl;
                return RecipeApp::Cli::EX_OK; // User cancelled, not an error
            }
        }

    } // namespace CliHandlers
} // namespace RecipeApp