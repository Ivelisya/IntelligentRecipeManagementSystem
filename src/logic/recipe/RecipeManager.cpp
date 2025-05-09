#include "RecipeManager.h"

#include <algorithm>  // For std::all_of, std::any_of
#include <iostream>   // For potential logging
#include <optional>
#include <vector>

#include "../../cli/ExitCodes.h"         // Include ExitCodes for exceptions
#include "../../domain/recipe/Recipe.h"  // Ensure Recipe is fully defined
#include "../../domain/recipe/RecipeRepository.h"

// Using declarations for convenience
using RecipeApp::Recipe;
using RecipeApp::Domain::Recipe::RecipeRepository;

namespace RecipeApp {

RecipeManager::RecipeManager(RecipeRepository &recipeRepository)
    : recipeRepository_(recipeRepository) {}

int RecipeManager::addRecipe(const Recipe &recipe_param) {
    std::cout << "[INFO] Adding recipe: " << recipe_param.getName()
              << std::endl;
    // 1. Check for name conflict using the repository
    // Assuming findByName returns a list/vector. If it's empty, no conflict.
    if (!recipeRepository_.findByName(recipe_param.getName(), false).empty()) {
        std::cerr << "[ERROR] Recipe name '" << recipe_param.getName()
                  << "' already exists." << std::endl;
        throw Cli::InvalidRecipeDataException(
            "Recipe name '" + recipe_param.getName() + "' already exists.");
        return -1;  // Name conflict
    }

    // 2. Create a new Recipe object using the builder
    auto builder = RecipeApp::Recipe::builder(0, recipe_param.getName())
                       .withIngredients(recipe_param.getIngredients())
                       .withSteps(recipe_param.getSteps())
                       .withCookingTime(recipe_param.getCookingTime())
                       .withDifficulty(recipe_param.getDifficulty())
                       .withCuisine(recipe_param.getCuisine())
                       .withTags(recipe_param.getTags());

    if (recipe_param.getNutritionalInfo().has_value()) {
        builder.withNutritionalInfo(recipe_param.getNutritionalInfo().value());
    }
    if (recipe_param.getImageUrl().has_value()) {
        builder.withImageUrl(recipe_param.getImageUrl().value());
    }
    RecipeApp::Recipe new_recipe_to_add = builder.build();

    // 3. Save using the repository and return the assigned ID (or -1 on
    // failure)
    try {
        int newRecipeId = recipeRepository_.save(new_recipe_to_add);
        std::cout << "[INFO] Recipe added successfully with ID: " << newRecipeId
                  << std::endl;
        return newRecipeId;
    } catch (const std::exception &e) {
        std::cerr << "[ERROR] Failed to save recipe: " << e.what() << std::endl;
        throw Cli::PersistenceException("Failed to save recipe: " +
                                        std::string(e.what()));
    }
}

// findRecipeByName now directly uses the repository method
std::vector<Recipe> RecipeManager::findRecipeByName(const std::string &name,
                                                    bool partialMatch) const {
    return recipeRepository_.findByName(name, partialMatch);
}

bool RecipeManager::deleteRecipe(int recipeId) {
    return recipeRepository_.remove(recipeId);
}

bool RecipeManager::updateRecipe(const Recipe &updated_recipe_param) {
    // 1. Check if the recipe exists
    std::optional<Recipe> existingRecipeOpt =
        recipeRepository_.findById(updated_recipe_param.getRecipeId());
    if (!existingRecipeOpt.has_value()) {
        // std::cerr << "RecipeManager: Recipe to update (ID: " <<
        // updated_recipe_param.getRecipeId() << ") not found." << std::endl;
        return false;  // Recipe not found
    }
    Recipe existingRecipe = existingRecipeOpt.value();

    // 2. Check for name conflict if the name has changed
    if (existingRecipe.getName() != updated_recipe_param.getName()) {
        // Check if the new name exists for any *other* recipe
        std::vector<Recipe> potentialConflicts =
            recipeRepository_.findByName(updated_recipe_param.getName(), false);
        for (const auto &conflict : potentialConflicts) {
            if (conflict.getRecipeId() != updated_recipe_param.getRecipeId()) {
                // std::cerr << "RecipeManager: Updated recipe name '" <<
                // updated_recipe_param.getName() << "' conflicts with existing
                // recipe (ID: " << conflict.getRecipeId() << ")." << std::endl;
                return false;  // Name conflict with another recipe
            }
        }
    }

    // 3. Save the updated recipe using the repository
    // The save method handles both create and update logic based on ID.
    return recipeRepository_.save(updated_recipe_param) != -1;
}

std::vector<Recipe> RecipeManager::getAllRecipes() const {
    return recipeRepository_.findAll();
}

// findRecipesByIngredients needs to iterate through all recipes from the
// repository
std::vector<Recipe> RecipeManager::findRecipesByIngredients(
    const std::vector<std::string> &requiredIngredients) const {
    if (requiredIngredients.empty()) {
        return {};  // Return empty if no ingredients specified
    }
    // The original logic implies matching ALL ingredients.
    return recipeRepository_.findByIngredients(requiredIngredients, true);
}

// findRecipesByCuisine needs to iterate through all recipes from the repository
std::vector<Recipe> RecipeManager::findRecipesByCuisine(
    const std::string &cuisine) const {
    if (cuisine.empty()) {
        return {};  // Return empty vector if cuisine is empty
    }
    return recipeRepository_.findByCuisine(cuisine);
}

std::optional<Recipe> RecipeManager::findRecipeById(int recipeId) const {
    return recipeRepository_.findById(recipeId);
}

// addRecipeDirectly and setNextRecipeId are removed as they are repository
// concerns.

void RecipeManager::addRecipeFromPersistence(const Recipe &recipe) {
    recipeRepository_.save(
        recipe);  // Assuming save handles both new and existing if ID is set
}

void RecipeManager::setNextRecipeIdFromPersistence(int nextId) {
    recipeRepository_.setNextId(nextId);
}

// --- New Tag-related Method Implementations ---

std::vector<Recipe> RecipeManager::findRecipesByTag(
    const std::string &tag) const {
    if (tag.empty()) {
        return {};  // Return empty vector if tag is empty
    }
    return recipeRepository_.findByTag(tag);
}

std::vector<Recipe> RecipeManager::findRecipesByTags(
    const std::vector<std::string> &tagsToFind, bool matchAll) const {
    if (tagsToFind.empty()) {
        return {};
    }
    return recipeRepository_.findByTags(tagsToFind, matchAll);
}

}  // namespace RecipeApp

std::vector<Recipe> RecipeManager::findRecipesByIds(
    const std::vector<int> &ids) const {
    if (ids.empty()) {
        return {};
    }
    return recipeRepository_.findManyByIds(ids);
}

// Add new method implementations at the end of the file, before the closing
// namespace brace.