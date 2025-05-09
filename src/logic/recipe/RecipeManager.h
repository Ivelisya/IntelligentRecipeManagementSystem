#ifndef RECIPE_MANAGER_H
#define RECIPE_MANAGER_H

#include "domain/recipe/Recipe.h"
#include "domain/recipe/RecipeRepository.h" // Added RecipeRepository include
#include <string>
#include <vector>
#include <optional> // For handling optional Recipe from repository

// Forward declaration for Recipe class if not fully included by RecipeRepository.h
// namespace RecipeApp { namespace Domain { namespace Recipe { class Recipe; } } }

namespace RecipeApp
{
    // Forward declare Recipe if it's in a nested namespace and not pulled in by RecipeRepository.h
    // For example:
    // namespace Domain { namespace Recipe { class Recipe; } }
    // using Domain::Recipe::Recipe;
    // However, Recipe.h is already included, so direct use of Recipe should be fine.

    class RecipeManager
    {
    private:
        Domain::Recipe::RecipeRepository &recipeRepository_; ///< Reference to the recipe repository

    public:
        /**
         * @brief 构造函数，接收一个 RecipeRepository 引用
         * @param recipeRepository 菜谱仓储的引用
         */
        explicit RecipeManager(Domain::Recipe::RecipeRepository &recipeRepository);

        /**
         * @brief 默认析构函数
         */
        ~RecipeManager() = default;

        /**
         * @brief 添加菜谱
         * @param recipe 待添加的菜谱对象 (其 ID 会被忽略)
         * @return 新分配的菜谱 ID (如果成功)，否则返回 -1 (例如，名称冲突)
         */
        int addRecipe(const Recipe &recipe_param);

        /**
         * @brief 根据名称查找菜谱
         * @param name 菜谱名称
         * @param partialMatch 是否部分匹配 (默认为 false)
         * @return 匹配菜谱的列表 (CustomLinkedList for now, might change to vector)
         */
        CustomDataStructures::CustomLinkedList<Recipe> findRecipeByName(const std::string &name, bool partialMatch = false) const;

        /**
         * @brief 根据食材组合查找菜谱（包含所有，精确匹配）
         * @param ingredients 食材名称列表
         * @return 匹配菜谱的列表
         */
        std::vector<Recipe> findRecipesByIngredients(const std::vector<std::string> &ingredients) const;

        /**
         * @brief 根据菜系分类查找菜谱
         * @param cuisine 菜系名称
         * @return 匹配菜谱的列表
         */
        std::vector<Recipe> findRecipesByCuisine(const std::string &cuisine) const;

        /**
         * @brief 删除菜谱
         * @param recipeId 菜谱ID
         * @return 是否删除成功
         */
        bool deleteRecipe(int recipeId);

        /**
         * @brief 更新菜谱
         * @param recipe 更新后的菜谱对象
         * @return 是否更新成功
         */
        bool updateRecipe(const Recipe &recipe);

        /**
         * @brief 获取所有菜谱
         * @return 所有菜谱的 std::vector
         */
        std::vector<Recipe> getAllRecipes() const;

        /**
         * @brief 根据ID查找菜谱
         * @param recipeId 菜谱ID
         * @return std::optional 包含找到的菜谱，如果未找到则为空
         */
        std::optional<Recipe> findRecipeById(int recipeId) const;
        // Removed non-const findRecipeById as direct modification via manager pointer is discouraged.
        // Updates should go through updateRecipe.

        /**
         * @brief Adds a recipe from persistence (bypasses normal add logic).
         * @param recipe The recipe object to add.
         */
        void addRecipeFromPersistence(const Recipe &recipe);

        /**
         * @brief Sets the next recipe ID from persistence.
         * @param nextId The next ID to be used by the repository.
         */
        void setNextRecipeIdFromPersistence(int nextId);

        // Methods like addRecipeDirectly, setNextRecipeId are removed
        // as their responsibilities are now handled by the RecipeRepository.
    };

} // namespace RecipeApp

#endif // RECIPE_MANAGER_H