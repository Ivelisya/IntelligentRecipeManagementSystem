#include "RecipeEncyclopediaManager.h"

#include <algorithm>  // For std::transform and std::search
#include <cctype>     // For std::tolower
#include <fstream>    // For std::ifstream
#include <iostream>   // For std::cerr (error logging)

namespace RecipeApp {
namespace Logic {
namespace Encyclopedia {

namespace {  // Anonymous namespace for helper functions
// Helper function to convert string to lowercase
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}
}  // end anonymous namespace

RecipeEncyclopediaManager::RecipeEncyclopediaManager() {
    // Constructor can be empty or initialize members if needed
}

RecipeEncyclopediaManager::~RecipeEncyclopediaManager() {
    // Destructor can be empty if no dynamic memory is managed directly by this
    // class
}

bool RecipeEncyclopediaManager::loadRecipes(const std::string& filepath) {
    encyclopediaRecipes
        .clear();  // Clear existing recipes before loading new ones

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[RecipeEncyclopediaManager] Error: Could not open file: "
                  << filepath << std::endl;
        return false;
    }

    try {
        nlohmann::json jsonData;
        file >> jsonData;  // Parse the JSON from the file stream

        if (jsonData.is_array()) {
            for (const auto& item : jsonData) {
                try {
                    // Assuming Recipe has a from_json method or nlohmann::json
                    // can convert it The adl_serializer for RecipeApp::Recipe
                    // should handle this.
                    RecipeApp::Recipe recipe =
                        item.get<RecipeApp::Recipe>();  // Use RecipeApp::Recipe
                    encyclopediaRecipes.push_back(recipe);
                } catch (const nlohmann::json::exception& e) {
                    std::cerr << "[RecipeEncyclopediaManager] Error parsing a "
                                 "recipe item: "
                              << e.what() << std::endl;
                    // Optionally, continue loading other recipes or return
                    // false
                }
            }
        } else {
            std::cerr << "[RecipeEncyclopediaManager] Error: JSON data is not "
                         "an array in file: "
                      << filepath << std::endl;
            return false;
        }
        std::cout << "[RecipeEncyclopediaManager] Successfully loaded "
                  << encyclopediaRecipes.size() << " recipes from " << filepath
                  << std::endl;
        return true;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[RecipeEncyclopediaManager] Error parsing JSON file "
                  << filepath << ": " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr
            << "[RecipeEncyclopediaManager] Generic error loading recipes from "
            << filepath << ": " << e.what() << std::endl;
        return false;
    }
    return false;  // Should not be reached if try-catch is exhaustive
}

const std::vector<RecipeApp::Recipe>& RecipeEncyclopediaManager::getAllRecipes()
    const {
    return encyclopediaRecipes;
}

bool RecipeEncyclopediaManager::containsCaseInsensitive(
    const std::string& text, const std::string& searchTerm) const {
    if (searchTerm.empty())
        return true;  // Or false, depending on desired behavior for empty
                      // search term
    if (text.empty()) return false;

    std::string lowerText = toLower(text);
    std::string lowerSearchTerm = toLower(searchTerm);

    return lowerText.find(lowerSearchTerm) != std::string::npos;
}

std::vector<RecipeApp::Recipe> RecipeEncyclopediaManager::searchRecipes(
    const std::string& searchTerm) const {
    std::vector<RecipeApp::Recipe> results;
    if (searchTerm.empty()) {
        return encyclopediaRecipes;  // Return all if search term is empty
    }

    for (const auto& recipe : encyclopediaRecipes) {
        // Search in name
        if (containsCaseInsensitive(recipe.getName(), searchTerm)) {
            results.push_back(recipe);
            continue;  // Found in name, no need to check other fields for this
                       // recipe
        }

        // Search in ingredients
        for (const auto& ingredient : recipe.getIngredients()) {
            if (containsCaseInsensitive(ingredient.name, searchTerm)) {
                results.push_back(recipe);
                goto next_recipe;  // Found in ingredients, move to next recipe
            }
        }

        // Search in tags
        for (const auto& tag : recipe.getTags()) {
            if (containsCaseInsensitive(tag, searchTerm)) {
                results.push_back(recipe);
                goto next_recipe;  // Found in tags, move to next recipe
            }
        }
    next_recipe:;  // Label for goto
    }
    return results;
}

std::optional<RecipeApp::Recipe> RecipeEncyclopediaManager::getRecipeById(
    int recipeId) const {
    for (const auto& recipe : encyclopediaRecipes) {
        if (recipe.getRecipeId() == recipeId) {
            return recipe;
        }
    }
    return std::nullopt;  // Return empty optional if not found
}

}  // namespace Encyclopedia
}  // namespace Logic
}  // namespace RecipeApp