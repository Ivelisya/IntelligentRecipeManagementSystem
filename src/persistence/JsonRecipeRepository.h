#ifndef JSON_RECIPE_REPOSITORY_H
#define JSON_RECIPE_REPOSITORY_H

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "JsonRepositoryBase.h"  // Include the new base class
#include "domain/recipe/Recipe.h"
#include "domain/recipe/RecipeRepository.h"
// json.hpp is included by JsonRepositoryBase.h

namespace RecipeApp {
namespace Persistence {

// using json = nlohmann::json; // Already in JsonRepositoryBase.h
// using RecipeApp::Difficulty; // Not directly used here, Recipe.h handles its
// own needs
using RecipeApp::Recipe;
using RecipeApp::Domain::Recipe::RecipeRepository;

class JsonRecipeRepository
    : public JsonRepositoryBase<Recipe>,
      virtual public RecipeRepository {  // virtual inheritance for
                                         // RecipeRepository
   public:
    explicit JsonRecipeRepository(const std::filesystem::path &baseDirectory,
                                  const std::string &fileName = "recipes.json");

    // ~JsonRecipeRepository() override = default; // If RecipeRepository has
    // virtual destructor

    // Publicly expose load from base if needed by external logic, or call it in
    // constructor
    bool load();  // This will call JsonRepositoryBase<Recipe>::load()

    // Implementation of RecipeRepository interface
    std::optional<RecipeApp::Recipe> findById(int recipeId) const override;
    std::vector<RecipeApp::Recipe> findAll() const override;
    int save(const RecipeApp::Recipe &recipe)
        override;  // This needs careful implementation
    bool remove(int recipeId) override;

    void setNextId(int nextId) override;
    int getNextId() const;  // Removed override, as it's not in the
                            // RecipeRepository interface

    // Recipe-specific finders
    std::vector<RecipeApp::Recipe> findByName(
        const std::string &name, bool partialMatch = false) const override;
    std::vector<RecipeApp::Recipe> findManyByIds(
        const std::vector<int> &ids) const override;
    std::vector<RecipeApp::Recipe> findByTag(
        const std::string &tagName) const override;
    std::vector<RecipeApp::Recipe> findByIngredients(
        const std::vector<std::string> &ingredientNames,
        bool matchAll) const override;
    std::vector<RecipeApp::Recipe> findByTags(
        const std::vector<std::string> &tagNames, bool matchAll) const override;

   private:
    // All members (m_filePath, m_items (as m_recipes), m_nextId,
    // m_jsonArrayKey) are now in JsonRepositoryBase<Recipe>. No private members
    // needed here unless for specific caching or state not in base.
};

}  // namespace Persistence
}  // namespace RecipeApp

#endif  // JSON_RECIPE_REPOSITORY_H