#ifndef RECIPE_REPOSITORY_H
#define RECIPE_REPOSITORY_H

#include <optional>
#include <string>
#include <vector>  // For findByName results

#include "domain/recipe/Recipe.h"

namespace RecipeApp {
namespace Domain {
namespace Recipe {
class RecipeRepository {
   public:
    virtual ~RecipeRepository() = default;

    virtual std::optional<RecipeApp::Recipe> findById(int recipeId) const = 0;
    // findByName might return multiple recipes if partial match is allowed
    virtual std::vector<RecipeApp::Recipe> findByName(
        const std::string &name, bool partialMatch = false) const = 0;
    virtual std::vector<RecipeApp::Recipe> findAll()
        const = 0;  // Changed return type
    virtual int save(
        const RecipeApp::Recipe &
            recipe) = 0;  // Changed to const ref, returns new/existing ID or -1
    virtual bool remove(int recipeId) = 0;
    virtual std::vector<RecipeApp::Recipe> findManyByIds(
        const std::vector<int> &ids) const = 0;
    virtual std::vector<RecipeApp::Recipe> findByTag(
        const std::string &tagName) const = 0;
    virtual std::vector<RecipeApp::Recipe> findByIngredients(
        const std::vector<std::string> &ingredientNames,
        bool matchAll) const = 0;
    virtual std::vector<RecipeApp::Recipe> findByTags(
        const std::vector<std::string> &tagNames, bool matchAll) const = 0;

    /**
     * @brief Sets the next available ID for a new recipe (for persistence
     * loading).
     * @param nextId The next ID to be used.
     */
    virtual void setNextId(int nextId) = 0;
    // ID generation might be handled here or by the specific implementation
    // (e.g., database sequence) The save method returning the ID helps manage
    // this.
};
}  // namespace Recipe
}  // namespace Domain
}  // namespace RecipeApp

#endif  // RECIPE_REPOSITORY_H