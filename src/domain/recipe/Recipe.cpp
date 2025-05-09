#include "Recipe.h"

#include <iostream>
#include <sstream>

#include "json.hpp"  // Include json header

// Convenience alias
using json = nlohmann::json;

namespace RecipeApp {

// --- JSON Serialization Implementation ---

// NLOHMANN_JSON_SERIALIZE_ENUM has been moved to Recipe.h

// Function definition for to_json (implementation moved from Recipe.h)
void to_json(json &j, const Recipe &recipe) {
    j = json{
        {"id", recipe.getRecipeId()},
        {"name", recipe.getName()},
        {"cookingTime", recipe.getCookingTime()},
        {"difficulty", recipe.getDifficulty()},
        {"tags", recipe.getTags()}  // Add tags serialization
    };

    // Serialize ingredients: std::vector<Ingredient>
    // nlohmann/json will automatically use Ingredient's to_json and from_json
    j["ingredients"] = recipe.getIngredients();

    // Serialize steps: vector<string> (nlohmann/json handles this directly)
    j["steps"] = recipe.getSteps();

    // Serialize optional fields
    if (recipe.getNutritionalInfo().has_value()) {
        j["nutritionalInfo"] = recipe.getNutritionalInfo().value();
    } else {
        j["nutritionalInfo"] = nullptr;
    }

    if (recipe.getImageUrl().has_value()) {
        j["imageUrl"] = recipe.getImageUrl().value();
    } else {
        j["imageUrl"] = nullptr;
    }
}

// The from_json function that took a Recipe& argument has been moved
// into the nlohmann::adl_serializer specialization in Recipe.h
// to allow direct construction via json_obj.get<Recipe>().

// --- Class Method Implementations ---

// The old public constructor implementation is removed as it's now private
// and its logic is handled by the RecipeBuilder or inline in the .h file.

RecipeBuilder Recipe::builder(int id, const std::string &name) {
    return RecipeBuilder(id, name);
}

std::string Recipe::getDisplayDetails() const {
    std::ostringstream oss;
    oss << "Recipe ID: " << recipeId << "\n";
    oss << "Name: " << name << "\n";

    oss << "Difficulty: ";
    switch (difficulty) {
        case Difficulty::Easy:
            oss << "Easy";
            break;
        case Difficulty::Medium:
            oss << "Medium";
            break;
        case Difficulty::Hard:
            oss << "Hard";
            break;
    }
    oss << "\n";

    oss << "Cooking Time: " << cookingTime << " minutes\n";

    oss << "Ingredients:\n";
    if (ingredients.empty()) {
        oss << "  (无配料信息)\n";
    } else {
        for (const auto &ingredient : ingredients) {
            oss << "  - " << ingredient.name << ": " << ingredient.quantity
                << "\n";
        }
    }

    oss << "Steps:\n";
    for (size_t i = 0; i < steps.size(); ++i) {
        oss << "  " << (i + 1) << ". " << steps[i] << "\n";
    }

    if (nutritionalInfo.has_value()) {
        oss << "Nutritional Info: " << nutritionalInfo.value() << "\n";
    }

    if (imageUrl.has_value()) {
        oss << "Image URL: " << imageUrl.value() << "\n";
    }

    if (!tags.empty()) {
        oss << "Tags: ";
        for (size_t i = 0; i < tags.size(); ++i) {
            oss << tags[i] << (i == tags.size() - 1 ? "" : ", ");
        }
        oss << "\n";
    }
    return oss.str();
}

bool Recipe::hasTag(const std::string &tagToCheck) const {
    for (const auto &tag : tags) {
        if (tag == tagToCheck) {
            return true;
        }
    }
    return false;
}

}  // namespace RecipeApp