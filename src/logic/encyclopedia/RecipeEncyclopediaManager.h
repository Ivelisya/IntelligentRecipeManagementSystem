#ifndef RECIPE_ENCYCLOPEDIA_MANAGER_H
#define RECIPE_ENCYCLOPEDIA_MANAGER_H

#include <memory>  // For std::unique_ptr if needed, or just raw pointers for now
#include <string>
#include <vector>
#include <optional> // For std::optional

#include "../../domain/recipe/Recipe.h"  // Assuming Recipe.h is in domain/recipe
#include "../../../include/json.hpp"     // For nlohmann::json (Corrected relative path)

// Forward declaration if RecipeRepository is used, though for encyclopedia it
// might be simpler namespace RecipeApp { namespace Domain { namespace Recipe {
// class RecipeRepository; } } }

namespace RecipeApp {
namespace Logic {
namespace Encyclopedia {

class RecipeEncyclopediaManager {
   public:
    RecipeEncyclopediaManager();
    ~RecipeEncyclopediaManager();

    /**
     * @brief Loads recipes from a JSON file.
     * @param filepath Path to the JSON file.
     * @return True if loading was successful, false otherwise.
     */
    bool loadRecipes(const std::string& filepath);

    /**
     * @brief Gets all recipes from the encyclopedia.
     * @return A const reference to the vector of recipes.
     */
    const std::vector<RecipeApp::Recipe>& getAllRecipes() const;

    /**
     * @brief Searches recipes in the encyclopedia based on a search term.
     *        The search term can match recipe name, ingredients, or tags.
     * @param searchTerm The string to search for.
     * @return A vector of recipes matching the search term.
     */
    std::vector<RecipeApp::Recipe> searchRecipes(
        const std::string& searchTerm) const;

    /**
     * @brief Gets a specific recipe by its ID.
     * @param recipeId The ID of the recipe to retrieve.
     * @return An std::optional<RecipeApp::Recipe> containing the recipe if found,
     *         otherwise an empty optional.
     */
    std::optional<RecipeApp::Recipe> getRecipeById(int recipeId) const;

   private:
    std::vector<RecipeApp::Recipe> encyclopediaRecipes;
    // Helper method for case-insensitive string comparison if needed for search
    bool containsCaseInsensitive(const std::string& text,
                                 const std::string& searchTerm) const;
};

}  // namespace Encyclopedia
}  // namespace Logic
}  // namespace RecipeApp

#endif  // RECIPE_ENCYCLOPEDIA_MANAGER_H