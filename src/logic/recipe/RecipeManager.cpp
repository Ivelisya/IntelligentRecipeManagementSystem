#include "logic/recipe/RecipeManager.h"
#include "domain/recipe/RecipeRepository.h"
#include "domain/recipe/Recipe.h" // Ensure Recipe is fully defined
#include <algorithm>              // For std::all_of, std::any_of
#include <vector>
#include <optional>
#include <iostream> // For potential logging

// Using declarations for convenience
using CustomDataStructures::CustomLinkedList; // Still needed for findByName return type
using RecipeApp::Recipe;
using RecipeApp::Domain::Recipe::RecipeRepository;

namespace RecipeApp
{

    RecipeManager::RecipeManager(RecipeRepository &recipeRepository)
        : recipeRepository_(recipeRepository)
    {
    }

    int RecipeManager::addRecipe(const Recipe &recipe_param)
    {
        // 1. Check for name conflict using the repository
        // Assuming findByName returns a list/vector. If it's empty, no conflict.
        if (!recipeRepository_.findByName(recipe_param.getName(), false).isEmpty())
        {
            // std::cerr << "RecipeManager: Recipe name '" << recipe_param.getName() << "' already exists." << std::endl;
            return -1; // Name conflict
        }

        // 2. Create a new Recipe object (ID will be assigned by repository)
        Recipe new_recipe_to_add(
            0, // ID 0 or -1 indicates new recipe for repository
            recipe_param.getName(),
            recipe_param.getIngredients(),
            recipe_param.getSteps(),
            recipe_param.getCookingTime(),
            recipe_param.getDifficulty(),
            recipe_param.getCuisine());

        // Copy optional fields
        if (recipe_param.getNutritionalInfo().has_value())
        {
            new_recipe_to_add.setNutritionalInfo(recipe_param.getNutritionalInfo().value());
        }
        if (recipe_param.getImageUrl().has_value())
        {
            new_recipe_to_add.setImageUrl(recipe_param.getImageUrl().value());
        }

        // 3. Save using the repository and return the assigned ID (or -1 on failure)
        return recipeRepository_.save(new_recipe_to_add);
    }

    // findRecipeByName now directly uses the repository method
    CustomLinkedList<Recipe> RecipeManager::findRecipeByName(const std::string &name, bool partialMatch) const
    {
        return recipeRepository_.findByName(name, partialMatch);
    }

    bool RecipeManager::deleteRecipe(int recipeId)
    {
        return recipeRepository_.remove(recipeId);
    }

    bool RecipeManager::updateRecipe(const Recipe &updated_recipe_param)
    {
        // 1. Check if the recipe exists
        std::optional<Recipe> existingRecipeOpt = recipeRepository_.findById(updated_recipe_param.getRecipeId());
        if (!existingRecipeOpt.has_value())
        {
            // std::cerr << "RecipeManager: Recipe to update (ID: " << updated_recipe_param.getRecipeId() << ") not found." << std::endl;
            return false; // Recipe not found
        }
        Recipe existingRecipe = existingRecipeOpt.value();

        // 2. Check for name conflict if the name has changed
        if (existingRecipe.getName() != updated_recipe_param.getName())
        {
            // Check if the new name exists for any *other* recipe
            CustomLinkedList<Recipe> potentialConflicts = recipeRepository_.findByName(updated_recipe_param.getName(), false);
            for (const auto &conflict : potentialConflicts)
            {
                if (conflict.getRecipeId() != updated_recipe_param.getRecipeId())
                {
                    // std::cerr << "RecipeManager: Updated recipe name '" << updated_recipe_param.getName() << "' conflicts with existing recipe (ID: " << conflict.getRecipeId() << ")." << std::endl;
                    return false; // Name conflict with another recipe
                }
            }
        }

        // 3. Save the updated recipe using the repository
        // The save method handles both create and update logic based on ID.
        return recipeRepository_.save(updated_recipe_param) != -1;
    }

    std::vector<Recipe> RecipeManager::getAllRecipes() const
    {
        return recipeRepository_.findAll();
    }

    // findRecipesByIngredients needs to iterate through all recipes from the repository
    std::vector<Recipe> RecipeManager::findRecipesByIngredients(const std::vector<std::string> &requiredIngredients) const
    {
        std::vector<Recipe> results;
        if (requiredIngredients.empty())
        {
            return results; // Return empty if no ingredients specified
        }

        std::vector<Recipe> allRecipes = recipeRepository_.findAll();
        for (const auto &recipe : allRecipes)
        {
            const auto &recipeIngredients = recipe.getIngredients(); // std::vector<std::pair<std::string, std::string>>

            // Check if *all* required ingredients are present in this recipe's ingredients
            bool allFound = std::all_of(requiredIngredients.begin(), requiredIngredients.end(),
                                        [&recipeIngredients](const std::string &reqIngredient)
                                        {
                                            // Check if any pair in recipeIngredients has reqIngredient as its first element
                                            return std::any_of(recipeIngredients.begin(), recipeIngredients.end(),
                                                               [&reqIngredient](const std::pair<std::string, std::string> &pair)
                                                               {
                                                                   return pair.first == reqIngredient; // Case-sensitive comparison for now
                                                               });
                                        });

            if (allFound)
            {
                results.push_back(recipe);
            }
        }
        return results;
    }

    // findRecipesByCuisine needs to iterate through all recipes from the repository
    std::vector<Recipe> RecipeManager::findRecipesByCuisine(const std::string &cuisine) const
    {
        std::vector<Recipe> results;
        if (cuisine.empty())
        {
            return results;
        }

        std::vector<Recipe> allRecipes = recipeRepository_.findAll();
        for (const auto &recipe : allRecipes)
        {
            // Case-sensitive comparison for now
            if (recipe.getCuisine() == cuisine)
            {
                results.push_back(recipe);
            }
        }
        return results;
    }

    std::optional<Recipe> RecipeManager::findRecipeById(int recipeId) const
    {
        return recipeRepository_.findById(recipeId);
    }

    // addRecipeDirectly and setNextRecipeId are removed as they are repository concerns.

} // namespace RecipeApp
// Add new method implementations at the end of the file, before the closing namespace brace.

void RecipeApp::RecipeManager::addRecipeFromPersistence(const Recipe &recipe)
{
    recipeRepository_.save(recipe); // Assuming save handles both new and existing if ID is set
}

void RecipeApp::RecipeManager::setNextRecipeIdFromPersistence(int nextId)
{
    recipeRepository_.setNextId(nextId);
}