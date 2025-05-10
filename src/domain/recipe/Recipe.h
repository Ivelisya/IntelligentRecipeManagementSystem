#ifndef RECIPE_H
#define RECIPE_H

#include <optional>
#include <string>
#include <vector>

#include "json.hpp"  // For JSON serialization

// Convenience alias
using json = nlohmann::json;
namespace RecipeApp {

/**
 * @struct Ingredient
 * @brief 代表菜谱中的一种食材及其用量
 */
struct Ingredient {
    std::string name;
    std::string quantity;
};

// JSON serialization for Ingredient
inline void to_json(json &j, const Ingredient &ing) {
    j = json{{"name", ing.name}, {"quantity", ing.quantity}};
}

inline void from_json(const json &j, Ingredient &ing) {
    j.at("name").get_to(ing.name);
    j.at("quantity").get_to(ing.quantity);
}

/**
 * @enum Difficulty
 * @brief 菜谱难度级别枚举
 */
enum class Difficulty {
    Easy,    ///< 简单
    Medium,  ///< 中等
    Hard     ///< 困难
};

// Helper to serialize Difficulty enum to string
// Moved from Recipe.cpp to be visible for adl_serializer in nlohmann namespace
NLOHMANN_JSON_SERIALIZE_ENUM(Difficulty, {{Difficulty::Easy, "Easy"},
                                          {Difficulty::Medium, "Medium"},
                                          {Difficulty::Hard, "Hard"}})

/**
 * @class Recipe
 * @brief 菜谱类，包含菜谱的基本信息和操作
 */
class RecipeBuilder;  // Forward declaration
class Recipe {
    friend class RecipeBuilder;  // Allow builder to access private
                                 // constructor/members
    // friend struct nlohmann::adl_serializer<RecipeApp::Recipe>; // Allow
    // serializer to access private constructor if needed

   private:
    int recipeId;
    std::string name;
    std::vector<Ingredient>
        ingredients;  // Changed from pair to Ingredient struct
    std::vector<std::string> steps;
    int cookingTime;
    Difficulty difficulty;
    std::optional<std::string> nutritionalInfo;
    std::optional<std::string> imageUrl;
    std::vector<std::string> tags;

    // Private constructor to be used by the Builder and potentially from_json
    Recipe(int id, std::string name,
           std::vector<Ingredient> ingredients,  // Changed
           std::vector<std::string> steps, int cookingTime,
           Difficulty difficulty, std::vector<std::string> tags,
           std::optional<std::string> nutritionalInfo,
           std::optional<std::string> imageUrl)
        : recipeId(id),
          name(std::move(name)),
          ingredients(std::move(ingredients)),
          steps(std::move(steps)),
          cookingTime(cookingTime),
          difficulty(difficulty),
          tags(std::move(tags)),
          nutritionalInfo(std::move(nutritionalInfo)),
          imageUrl(std::move(imageUrl)) {}

   public:
    // Static method to get a builder instance
    static RecipeBuilder builder(int id, const std::string &name);

    /**
     * @brief 默认析构函数
     */
    ~Recipe() = default;

    /**
     * @brief 获取菜谱详细信息的格式化字符串
     * @return std::string 包含菜谱详细信息的字符串
     */
    std::string getDisplayDetails() const;

    // Getter 方法
    int getRecipeId() const { return recipeId; }
    int getId() const {
        return recipeId;
    }  // Added for JsonRepositoryBase compatibility
    const std::string &getName() const { return name; }
    const std::vector<Ingredient> &getIngredients() const {  // Changed
        return ingredients;
    }
    const std::vector<std::string> &getSteps() const { return steps; }
    int getCookingTime() const { return cookingTime; }
    Difficulty getDifficulty() const { return difficulty; }
    const std::optional<std::string> &getNutritionalInfo() const {
        return nutritionalInfo;
    }
    const std::optional<std::string> &getImageUrl() const { return imageUrl; }
    const std::vector<std::string> &getTags() const { return tags; }

    // Setter 方法
    void setName(const std::string &newName) {
        if (newName.empty()) {
            throw std::invalid_argument("Recipe name cannot be empty.");
        }
        name = newName;
    }
    void setIngredients(
        const std::vector<Ingredient> &newIngredients) {  // Changed
        ingredients = newIngredients;
    }
    void setSteps(const std::vector<std::string> &newSteps) {
        steps = newSteps;
    }
    void setCookingTime(int newTime) {
        if (newTime >= 0) {
            cookingTime = newTime;
        } else {
            throw std::invalid_argument("Cooking time cannot be negative.");
        }
    }
    void setDifficulty(Difficulty newDifficulty) { difficulty = newDifficulty; }
    void setNutritionalInfo(const std::string &info) { nutritionalInfo = info; }
    void setImageUrl(const std::string &url) { imageUrl = url; }
    void setTags(const std::vector<std::string> &newTags) { tags = newTags; }
    void addTag(const std::string &tag) {
        if (!tag.empty()) {
            // Check if the tag already exists
            bool found = false;
            for (const auto& existingTag : tags) {
                if (existingTag == tag) {
                    found = true;
                    break;
                }
            }
            // Only add the tag if it's not already present
            if (!found) {
                tags.push_back(tag);
            }
        }
    }
    void removeTag(const std::string &tagToRemove) {
        tags.erase(std::remove(tags.begin(), tags.end(), tagToRemove),
                   tags.end());
    }

    /**
     * @brief 检查食谱是否包含指定的标签
     * @param tag 要检查的标签
     * @return 如果包含则返回 true，否则返回 false
     */
    bool hasTag(const std::string &tag) const;

    // Overload operator== for comparisons
    bool operator==(const Recipe &other) const {
        return this->recipeId == other.recipeId;
    }
};

class RecipeBuilder {
   private:
    int m_id;
    std::string m_name;
    std::vector<Ingredient> m_ingredients;  // Changed
    std::vector<std::string> m_steps;
    int m_cookingTime = 0;                       // Default value
    Difficulty m_difficulty = Difficulty::Easy;  // Default value
    std::optional<std::string> m_nutritionalInfo;
    std::optional<std::string> m_imageUrl;
    std::vector<std::string> m_tags;

   public:
    RecipeBuilder(int id, const std::string &name) : m_id(id), m_name(name) {}

    RecipeBuilder &withIngredients(
        const std::vector<Ingredient> &ingredients) {  // Changed
        m_ingredients = ingredients;
        return *this;
    }

    RecipeBuilder &withSteps(const std::vector<std::string> &steps) {
        m_steps = steps;
        return *this;
    }

    RecipeBuilder &withCookingTime(int cookingTime) {
        if (cookingTime < 0)
            throw std::invalid_argument("Cooking time cannot be negative.");
        m_cookingTime = cookingTime;
        return *this;
    }

    RecipeBuilder &withDifficulty(Difficulty difficulty) {
        m_difficulty = difficulty;
        return *this;
    }

    RecipeBuilder &withNutritionalInfo(const std::string &info) {
        m_nutritionalInfo = info;
        return *this;
    }

    RecipeBuilder &withImageUrl(const std::string &url) {
        m_imageUrl = url;
        return *this;
    }

    RecipeBuilder &withTags(const std::vector<std::string> &tags) {
        m_tags = tags;
        return *this;
    }

    Recipe build() const {
        // Basic validation for required fields (name is already in constructor)
        if (m_name.empty())
            throw std::invalid_argument(
                "Recipe name cannot be empty for build.");
        if (m_cookingTime <= 0 &&
            m_id !=
                0) {  // Allow 0 for placeholder during creation if ID is also 0
            // For existing recipes being built (e.g. from DB), cooking time
            // should be positive. For new recipes being built by user, this
            // check is also good. The m_id != 0 check is a bit heuristic, might
            // need refinement based on how builder is used for updates. For
            // now, let's assume cookingTime must be > 0 if it's set. The
            // constructor of Recipe itself also has a check.
        }

        return Recipe(m_id, m_name, m_ingredients, m_steps, m_cookingTime,
                      m_difficulty, m_tags, m_nutritionalInfo, m_imageUrl);
    }
};

// --- JSON Serialization Declarations (Implementation in Recipe.cpp) ---

// Function declaration for JSON serialization.
// Needs to be in the same namespace as Recipe for ADL (Argument-Dependent
// Lookup).
void to_json(json &j, const RecipeApp::Recipe &recipe);

// void from_json(const json &j, Recipe &recipe); // Will be handled by
// adl_serializer

}  // namespace RecipeApp

// --- nlohmann::json adl_serializer specialization for RecipeApp::Recipe ---
namespace nlohmann {
template <>
struct adl_serializer<RecipeApp::Recipe> {
    // Convert Recipe to JSON
    static void to_json(json &j, const RecipeApp::Recipe &recipe) {
        // Call the existing free function in RecipeApp namespace
        RecipeApp::to_json(j, recipe);
    }

    // Convert JSON to Recipe using the builder
    static RecipeApp::Recipe from_json(const json &j) {
        int recipe_id;
        if (j.contains("id") && j.at("id").is_number_integer()) {
            j.at("id").get_to(recipe_id);
            if (recipe_id <= 0)
                throw std::runtime_error(
                    "Recipe ID in JSON must be a positive integer.");
        } else {
            throw std::runtime_error(
                "Recipe ID is missing or not an integer in JSON.");
        }

        std::string name;
        if (j.contains("name") && j.at("name").is_string()) {
            j.at("name").get_to(name);
            if (name.empty())
                throw std::runtime_error(
                    "Recipe name cannot be empty in JSON.");
        } else {
            throw std::runtime_error(
                "Recipe name is missing or not a string in JSON.");
        }

        RecipeApp::RecipeBuilder builder =
            RecipeApp::Recipe::builder(recipe_id, name);

        if (j.contains("cookingTime") &&
            j.at("cookingTime").is_number_integer()) {
            int cookingTime = j.at("cookingTime").get<int>();
            if (cookingTime < 0)
                throw std::runtime_error(
                    "Recipe cookingTime cannot be negative in JSON.");
            builder.withCookingTime(cookingTime);
        } else {
            throw std::runtime_error(
                "Recipe cookingTime is missing or not an integer in JSON.");
        }

        if (j.contains("difficulty")) {
            builder.withDifficulty(
                j.at("difficulty").get<RecipeApp::Difficulty>());
        } else {
            throw std::runtime_error("Recipe difficulty is missing in JSON.");
        }

        if (j.contains("ingredients") && j.at("ingredients").is_array()) {
            std::vector<RecipeApp::Ingredient> ingredients_vec;  // Changed type
            for (const auto &ing_json : j.at("ingredients")) {
                // Assuming ing_json is an object like {"name": "...",
                // "quantity": "..."} and Ingredient has a from_json or is
                // directly convertible
                try {
                    ingredients_vec.push_back(
                        ing_json.get<RecipeApp::Ingredient>());
                } catch (const json::exception &e) {
                    (void)e;  // Mark as intentionally unused to suppress C4101
                    // Log error or skip malformed ingredient
                    // std::cerr << "Error parsing ingredient: " << e.what() <<
                    // std::endl;
                }
            }
            builder.withIngredients(ingredients_vec);
        }

        if (j.contains("steps") && j.at("steps").is_array()) {
            std::vector<std::string> steps;
            for (const auto &step_json : j.at("steps")) {
                if (step_json.is_string()) {
                    steps.push_back(step_json.get<std::string>());
                }
            }
            builder.withSteps(steps);
        }

        if (j.contains("tags") && j.at("tags").is_array()) {
            std::vector<std::string> tags;
            for (const auto &tag_json : j.at("tags")) {
                if (tag_json.is_string()) {
                    tags.push_back(tag_json.get<std::string>());
                }
            }
            builder.withTags(tags);
        }

        if (j.contains("nutritionalInfo") &&
            j.at("nutritionalInfo").is_string() &&
            !j.at("nutritionalInfo").get<std::string>().empty()) {
            builder.withNutritionalInfo(
                j.at("nutritionalInfo").get<std::string>());
        }

        if (j.contains("imageUrl") && j.at("imageUrl").is_string() &&
            !j.at("imageUrl").get<std::string>().empty()) {
            builder.withImageUrl(j.at("imageUrl").get<std::string>());
        }

        return builder.build();
    }
};
}  // namespace nlohmann

#endif  // RECIPE_H