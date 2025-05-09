#include "Recipe.h"
#include <iostream>
#include <sstream>
#include "json.hpp" // Include json header

// Convenience alias
using json = nlohmann::json;

namespace RecipeApp
{

    // --- JSON Serialization Implementation ---

    // NLOHMANN_JSON_SERIALIZE_ENUM has been moved to Recipe.h

    // Function definition for to_json (implementation moved from Recipe.h)
    void to_json(json &j, const Recipe &recipe)
    {
        j = json{
            {"id", recipe.getRecipeId()},
            {"name", recipe.getName()},
            {"cuisine", recipe.getCuisine()},
            {"cookingTime", recipe.getCookingTime()},
            {"difficulty", recipe.getDifficulty()},
            {"tags", recipe.getTags()} // Add tags serialization
        };

        // Serialize ingredients: vector<pair<string, string>>
        json ingredients_array = json::array();
        for (const auto &ing_pair : recipe.getIngredients())
        {
            ingredients_array.push_back({{"name", ing_pair.first},
                                         {"quantity", ing_pair.second}});
        }
        j["ingredients"] = ingredients_array;

        // Serialize steps: vector<string> (nlohmann/json handles this directly)
        j["steps"] = recipe.getSteps();

        // Serialize optional fields
        if (recipe.getNutritionalInfo().has_value())
        {
            j["nutritionalInfo"] = recipe.getNutritionalInfo().value();
        }
        else
        {
            j["nutritionalInfo"] = nullptr;
        }

        if (recipe.getImageUrl().has_value())
        {
            j["imageUrl"] = recipe.getImageUrl().value();
        }
        else
        {
            j["imageUrl"] = nullptr;
        }
    }

    // The from_json function that took a Recipe& argument has been moved
    // into the nlohmann::adl_serializer specialization in Recipe.h
    // to allow direct construction via json_obj.get<Recipe>().

    // --- Class Method Implementations ---

    Recipe::Recipe(int id, const std::string &name, const std::vector<std::pair<std::string, std::string>> &ingredients,
                   const std::vector<std::string> &steps, int cookingTime, Difficulty difficulty, const std::string &cuisine, const std::vector<std::string> &tags)
        : recipeId(id), name(name), ingredients(ingredients), steps(steps), cookingTime(cookingTime),
          difficulty(difficulty), cuisine(cuisine), tags(tags) // Initialize tags
    {
        // Consistent with setCookingTime, throw if cookingTime is invalid
        if (this->cookingTime < 0)
        { // Use this->cookingTime as it's already initialized by member initializer list
            throw std::invalid_argument("Cooking time cannot be negative during construction.");
        }
    }

    std::string Recipe::getDisplayDetails() const
    {
        std::ostringstream oss;
        oss << "Recipe ID: " << recipeId << "\n";
        oss << "Name: " << name << "\n";
        oss << "Cuisine: " << cuisine << "\n";

        oss << "Difficulty: ";
        switch (difficulty)
        {
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
        for (const auto &ingredient : ingredients)
        {
            oss << "  - " << ingredient.first << ": " << ingredient.second << "\n";
        }

        oss << "Steps:\n";
        for (size_t i = 0; i < steps.size(); ++i)
        {
            oss << "  " << (i + 1) << ". " << steps[i] << "\n";
        }

        if (nutritionalInfo.has_value())
        {
            oss << "Nutritional Info: " << nutritionalInfo.value() << "\n";
        }

        if (imageUrl.has_value())
        {
            oss << "Image URL: " << imageUrl.value() << "\n";
        }

        if (!tags.empty())
        {
            oss << "Tags: ";
            for (size_t i = 0; i < tags.size(); ++i)
            {
                oss << tags[i] << (i == tags.size() - 1 ? "" : ", ");
            }
            oss << "\n";
        }
        return oss.str();
    }

    bool Recipe::hasTag(const std::string &tagToCheck) const
    {
        for (const auto &tag : tags)
        {
            if (tag == tagToCheck)
            {
                return true;
            }
        }
        return false;
    }

} // namespace RecipeApp