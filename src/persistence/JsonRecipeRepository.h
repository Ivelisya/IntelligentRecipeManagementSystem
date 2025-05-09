#ifndef JSON_RECIPE_REPOSITORY_H
#define JSON_RECIPE_REPOSITORY_H

#include "domain/recipe/RecipeRepository.h"
#include "domain/recipe/Recipe.h"
#include "core/CustomLinkedList.h"
#include "json.hpp" // nlohmann::json
#include <string>
#include <fstream>
#include <vector>
#include <optional>

namespace RecipeApp
{
    namespace Persistence
    {

        using json = nlohmann::json;
        using CustomDataStructures::CustomLinkedList;
        // Corrected using declarations: Recipe and Difficulty are in RecipeApp namespace directly
        using RecipeApp::Difficulty;
        using RecipeApp::Recipe;
        using RecipeApp::Domain::Recipe::RecipeRepository; // This one is correct

        class JsonRecipeRepository : public RecipeRepository
        {
        public:
            explicit JsonRecipeRepository(const std::string &filePath);

            // Load/Save operations
            bool load();
            bool saveAll();

            // Implementation of RecipeRepository interface
            std::optional<Recipe> findById(int recipeId) const override;
            CustomLinkedList<Recipe> findByName(const std::string &name, bool partialMatch = false) const override;
            std::vector<Recipe> findAll() const override; // Changed to match interface
            int save(const Recipe &recipe) override;      // Changed to const ref, matches interface
            bool remove(int recipeId) override;
            void setNextId(int nextId) override;

            // Helper to get the next available recipe ID
            int getNextId() const; // This should also be override if in base

        private:
            std::string m_filePath;
            CustomLinkedList<Recipe> m_recipes; // In-memory storage
            int m_nextId = 1;                   // Simple counter for new IDs
        };

    } // namespace Persistence
} // namespace RecipeApp

#endif // JSON_RECIPE_REPOSITORY_H