#ifndef RECIPE_H
#define RECIPE_H

#include <string>
#include <vector>
#include <optional>

#include "json.hpp" // For JSON serialization

// Convenience alias
using json = nlohmann::json;
namespace RecipeApp
{

    /**
     * @enum Difficulty
     * @brief 菜谱难度级别枚举
     */
    enum class Difficulty
    {
        Easy,   ///< 简单
        Medium, ///< 中等
        Hard    ///< 困难
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
    class Recipe
    {
    private:
        int recipeId;                                                 ///< 菜谱ID，唯一标识
        std::string name;                                             ///< 菜谱名称，唯一
        std::vector<std::pair<std::string, std::string>> ingredients; ///< 食材列表，包含名称和数量
        std::vector<std::string> steps;                               ///< 烹饪步骤
        int cookingTime;                                              ///< 烹饪时间（分钟）
        Difficulty difficulty;                                        ///< 难度级别
        std::string cuisine;                                          ///< 菜系
        std::optional<std::string> nutritionalInfo;                   ///< 营养信息（可选）
        std::optional<std::string> imageUrl;                          ///< 图片URL（可选）
        std::vector<std::string> tags;                                ///< 食谱标签

    public:
        /**
         * @brief 构造函数
         * @param id 菜谱ID
         * @param name 菜谱名称
         * @param ingredients 食材列表
         * @param steps 烹饪步骤
         * @param cookingTime 烹饪时间（分钟）
         * @param difficulty 难度级别
         * @param cuisine 菜系
         * @param tags 食谱标签
         */
        Recipe(int id, const std::string &name, const std::vector<std::pair<std::string, std::string>> &ingredients,
               const std::vector<std::string> &steps, int cookingTime, Difficulty difficulty, const std::string &cuisine, const std::vector<std::string> &tags = {});

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
        const std::string &getName() const { return name; }
        const std::vector<std::pair<std::string, std::string>> &getIngredients() const { return ingredients; }
        const std::vector<std::string> &getSteps() const { return steps; }
        int getCookingTime() const { return cookingTime; }
        Difficulty getDifficulty() const { return difficulty; }
        const std::string &getCuisine() const { return cuisine; }
        const std::optional<std::string> &getNutritionalInfo() const { return nutritionalInfo; }
        const std::optional<std::string> &getImageUrl() const { return imageUrl; }
        const std::vector<std::string> &getTags() const { return tags; }

        // Setter 方法
        void setName(const std::string &newName)
        {
            if (newName.empty())
            {
                throw std::invalid_argument("Recipe name cannot be empty.");
            }
            name = newName;
        }
        void setIngredients(const std::vector<std::pair<std::string, std::string>> &newIngredients) { ingredients = newIngredients; }
        void setSteps(const std::vector<std::string> &newSteps) { steps = newSteps; }
        void setCookingTime(int newTime)
        {
            if (newTime >= 0)
            {
                cookingTime = newTime;
            }
            else
            {
                throw std::invalid_argument("Cooking time cannot be negative.");
            }
        }
        void setDifficulty(Difficulty newDifficulty) { difficulty = newDifficulty; }
        void setCuisine(const std::string &newCuisine)
        {
            if (newCuisine.empty())
            {
                throw std::invalid_argument("Cuisine cannot be empty.");
            }
            cuisine = newCuisine;
        }
        void setNutritionalInfo(const std::string &info) { nutritionalInfo = info; }
        void setImageUrl(const std::string &url) { imageUrl = url; }
        void setTags(const std::vector<std::string> &newTags) { tags = newTags; }
        void addTag(const std::string &tag)
        {
            if (!tag.empty())
            {
                // Avoid adding duplicate tags if needed, or allow them
                // For now, let's allow duplicates as std::vector doesn't enforce uniqueness
                tags.push_back(tag);
            }
        }
        void removeTag(const std::string &tagToRemove)
        {
            tags.erase(std::remove(tags.begin(), tags.end(), tagToRemove), tags.end());
        }

        /**
         * @brief 检查食谱是否包含指定的标签
         * @param tag 要检查的标签
         * @return 如果包含则返回 true，否则返回 false
         */
        bool hasTag(const std::string &tag) const;

        // Overload operator== for CustomLinkedList operations
        bool operator==(const Recipe &other) const
        {
            return this->recipeId == other.recipeId;
        }
    };

    // --- JSON Serialization Declarations (Implementation in Recipe.cpp) ---

    // Function declaration for JSON serialization.
    // Needs to be in the same namespace as Recipe for ADL (Argument-Dependent Lookup).
    void to_json(json &j, const Recipe &recipe);

    // void from_json(const json &j, Recipe &recipe); // Will be handled by adl_serializer

} // namespace RecipeApp

// --- nlohmann::json adl_serializer specialization for RecipeApp::Recipe ---
namespace nlohmann
{
    template <>
    struct adl_serializer<RecipeApp::Recipe>
    {
        // Convert Recipe to JSON
        static void to_json(json &j, const RecipeApp::Recipe &recipe)
        {
            // Call the existing free function in RecipeApp namespace
            RecipeApp::to_json(j, recipe);
        }

        // Convert JSON to Recipe
        static RecipeApp::Recipe from_json(const json &j)
        {
            int recipe_id;
            if (j.contains("id") && j.at("id").is_number_integer())
            {
                j.at("id").get_to(recipe_id);
                if (recipe_id <= 0)
                {
                    throw std::runtime_error("Recipe ID in JSON must be a positive integer.");
                }
            }
            else
            {
                throw std::runtime_error("Recipe ID is missing or not an integer in JSON.");
            }

            std::string name;
            if (j.contains("name") && j.at("name").is_string())
            {
                j.at("name").get_to(name);
                if (name.empty())
                {
                    throw std::runtime_error("Recipe name cannot be empty in JSON.");
                }
            }
            else
            {
                throw std::runtime_error("Recipe name is missing or not a string in JSON.");
            }

            std::string cuisine;
            if (j.contains("cuisine") && j.at("cuisine").is_string())
            {
                j.at("cuisine").get_to(cuisine);
            }
            else
            {
                // Allow cuisine to be missing or default it, or throw if strictly required
                // For now, let's assume it's required as per original from_json logic
                throw std::runtime_error("Recipe cuisine is missing or not a string in JSON.");
            }

            int cookingTime;
            if (j.contains("cookingTime") && j.at("cookingTime").is_number_integer())
            {
                j.at("cookingTime").get_to(cookingTime);
                if (cookingTime < 0)
                {
                    throw std::runtime_error("Recipe cookingTime cannot be negative in JSON.");
                }
            }
            else
            {
                throw std::runtime_error("Recipe cookingTime is missing or not an integer in JSON.");
            }

            RecipeApp::Difficulty difficulty;
            if (j.contains("difficulty"))
            {
                // NLOHMANN_JSON_SERIALIZE_ENUM handles the conversion for Difficulty
                j.at("difficulty").get_to(difficulty);
            }
            else
            {
                throw std::runtime_error("Recipe difficulty is missing in JSON.");
            }

            std::vector<std::pair<std::string, std::string>> ingredients;
            if (j.contains("ingredients") && j.at("ingredients").is_array())
            {
                for (const auto &ing_json : j.at("ingredients"))
                {
                    if (ing_json.is_object() && ing_json.contains("name") && ing_json.at("name").is_string() &&
                        ing_json.contains("quantity") && ing_json.at("quantity").is_string())
                    {
                        ingredients.emplace_back(ing_json.at("name").get<std::string>(), ing_json.at("quantity").get<std::string>());
                    }
                    else
                    {
                        // Optionally throw an error for malformed ingredient
                    }
                }
            }

            std::vector<std::string> steps;
            if (j.contains("steps") && j.at("steps").is_array())
            {
                for (const auto &step_json : j.at("steps"))
                {
                    if (step_json.is_string())
                    {
                        steps.push_back(step_json.get<std::string>());
                    }
                }
            }

            std::vector<std::string> tags;
            if (j.contains("tags") && j.at("tags").is_array())
            {
                for (const auto &tag_json : j.at("tags"))
                {
                    if (tag_json.is_string())
                    {
                        tags.push_back(tag_json.get<std::string>());
                    }
                }
            }

            RecipeApp::Recipe recipe(recipe_id, name, ingredients, steps, cookingTime, difficulty, cuisine, tags);

            if (j.contains("nutritionalInfo") && !j.at("nutritionalInfo").is_null())
            {
                if (j.at("nutritionalInfo").is_string())
                {
                    recipe.setNutritionalInfo(j.at("nutritionalInfo").get<std::string>());
                }
            }

            if (j.contains("imageUrl") && !j.at("imageUrl").is_null())
            {
                if (j.at("imageUrl").is_string())
                {
                    recipe.setImageUrl(j.at("imageUrl").get<std::string>());
                }
            }
            return recipe;
        }
    };
} // namespace nlohmann

#endif // RECIPE_H