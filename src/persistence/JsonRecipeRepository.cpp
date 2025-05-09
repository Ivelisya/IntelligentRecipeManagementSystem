#include "persistence/JsonRecipeRepository.h"
#include "domain/recipe/Recipe.h" // Ensure Recipe class definition is included
#include "json.hpp"               // For nlohmann::json
#include <fstream>
#include <iostream> // Required for std::cout, std::endl
#include <stdexcept>
#include <sstream>   // For string stream helpers
#include <algorithm> // For std::find_if, std::remove_if

namespace RecipeApp
{
    namespace Persistence
    {
        // Corrected using declarations for types directly under RecipeApp
        using RecipeApp::Difficulty;
        using RecipeApp::Recipe;
        // Keep using for the repository interface itself if needed, or use full qualification
        // using RecipeApp::Domain::Recipe::RecipeRepository;

        // Constructor
        JsonRecipeRepository::JsonRecipeRepository(const std::string &filePath)
            : m_filePath(filePath), m_nextId(1) {}

        // --- Load/Save Operations ---

        bool JsonRecipeRepository::load()
        {
            std::ifstream file(m_filePath);
            if (!file.is_open())
            {
                std::cerr << "Warning: Could not open recipe data file for reading: " << m_filePath << ". Starting with an empty recipe list." << std::endl;
                m_recipes.clearList();
                m_nextId = 1;
                return true;
            }

            try
            {
                json data = json::parse(file);
                file.close();

                m_recipes.clearList();
                int maxId = 0;

                if (data.contains("recipes") && data["recipes"].is_array())
                {
                    for (const auto &recipeJson : data["recipes"])
                    {
                        try
                        {
                            // Use Recipe's from_json via nlohmann/json's get<Recipe>()
                            Recipe recipe = recipeJson.get<Recipe>();
                            if (recipe.getRecipeId() > 0)
                            {
                                m_recipes.addBack(recipe);
                                if (recipe.getRecipeId() > maxId)
                                {
                                    maxId = recipe.getRecipeId();
                                }
                            }
                            else
                            {
                                std::cerr << "Warning: Invalid recipe ID (<=0) found while loading. Skipped: " << recipeJson.dump() << std::endl;
                            }
                        }
                        catch (const std::exception &e)
                        {
                            std::cerr << "Warning: Failed to load a recipe due to error: " << e.what() << ". Invalid recipe JSON: " << recipeJson.dump(2) << std::endl;
                            // Continue to the next recipe
                        }
                    }
                }
                m_nextId = maxId + 1;
                return true;
            }
            catch (const json::parse_error &e)
            {
                std::cerr << "Error: Failed to parse recipe JSON data: " << e.what() << " at byte " << e.byte << std::endl;
                m_recipes.clearList();
                m_nextId = 1;
                return false;
            }
            catch (const std::exception &e) // This will now primarily catch issues outside the recipe iteration loop
            {
                std::cerr << "Error loading recipe data (outer scope): " << e.what() << std::endl;
                m_recipes.clearList();
                m_nextId = 1;
                return false;
            }
        }

        bool JsonRecipeRepository::saveAll()
        {
            json data;
            json recipesJson = json::array();

            for (const auto &recipe : m_recipes)
            {
                // Use Recipe's to_json via nlohmann/json's constructor or assignment
                recipesJson.push_back(json(recipe));
            }
            data["recipes"] = recipesJson;

            std::ofstream file(m_filePath);
            if (!file.is_open())
            {
                std::cerr << "Error: Could not open recipe data file for writing: " << m_filePath << std::endl;
                return false;
            }

            try
            {
                file << data.dump(2); // Pretty print
                file.close();
                return true;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error saving recipe data: " << e.what() << std::endl;
                if (file.is_open())
                    file.close();
                return false;
            }
        }

        // --- RecipeRepository Interface Implementation ---

        std::optional<Recipe> JsonRecipeRepository::findById(int recipeId) const
        {
            for (const auto &recipe : m_recipes)
            {
                if (recipe.getRecipeId() == recipeId)
                {
                    return recipe; // Return a copy
                }
            }
            return std::nullopt;
        }
        CustomLinkedList<Recipe> JsonRecipeRepository::findByName(const std::string &name, bool partialMatch) const
        {
            std::cout << "[DEBUG] findByName called with: " << name << ", partialMatch: " << partialMatch << std::endl; // DEBUG
            CustomLinkedList<Recipe> results;

            // 转换查询字符串为小写，实现不区分大小写的搜索
            std::string lowerName = name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

            std::cout << "[DEBUG] m_recipes size: " << m_recipes.getSize() << std::endl; // DEBUG
            int count = 0;                                                               // DEBUG
            for (const auto &recipe : m_recipes)
            {
                std::cout << "[DEBUG] Iterating recipe " << count++ << ", ID: " << recipe.getRecipeId() << std::endl; // DEBUG
                std::string currentRecipeName = recipe.getName();
                std::cout << "[DEBUG] Recipe name: " << currentRecipeName << std::endl; // DEBUG

                // 转换当前菜谱名称为小写，实现不区分大小写的比较
                std::string lowerCurrentName = currentRecipeName;
                std::transform(lowerCurrentName.begin(), lowerCurrentName.end(), lowerCurrentName.begin(), ::tolower);

                if (partialMatch)
                {
                    if (lowerCurrentName.find(lowerName) != std::string::npos)
                    {
                        results.addBack(recipe);
                    }
                }
                else
                {
                    if (lowerCurrentName == lowerName)
                    {
                        results.addBack(recipe);
                    }
                }
            }
            std::cout << "[DEBUG] findByName finished iteration. Results size: " << results.getSize() << std::endl; // DEBUG
            return results;
        }

        std::vector<Recipe> JsonRecipeRepository::findAll() const
        {
            std::vector<Recipe> allRecipes;
            allRecipes.reserve(m_recipes.getSize()); // Optional: pre-allocate memory
            for (const auto &recipe : m_recipes)
            {
                allRecipes.push_back(recipe); // Add a copy of each recipe
            }
            return allRecipes;
        }

        int JsonRecipeRepository::save(const Recipe &recipeToSave)
        {
            int idToUse = recipeToSave.getRecipeId();
            bool existingFoundAndUpdated = false;

            if (idToUse > 0)
            { // If a specific ID is provided, try to find and update
                for (auto it = m_recipes.begin(); it != m_recipes.end(); ++it)
                {
                    if ((*it).getRecipeId() == idToUse)
                    {
                        *it = recipeToSave; // Update existing recipe
                        existingFoundAndUpdated = true;
                        break;
                    }
                }
            }

            if (!existingFoundAndUpdated)
            {                                      // If not found (either ID was <=0, or ID > 0 but not in list - e.g. loading from persistence)
                Recipe recipeToAdd = recipeToSave; // Start with a copy

                if (idToUse <= 0)
                { // It's a truly new recipe, assign a new ID from m_nextId
                    idToUse = m_nextId++;
                    // Create a new Recipe object with the new ID and copy other fields
                    recipeToAdd = Recipe(idToUse,
                                         recipeToSave.getName(),
                                         recipeToSave.getIngredients(),
                                         recipeToSave.getSteps(),
                                         recipeToSave.getCookingTime(),
                                         recipeToSave.getDifficulty(),
                                         recipeToSave.getCuisine());
                    if (recipeToSave.getNutritionalInfo().has_value())
                    {
                        recipeToAdd.setNutritionalInfo(recipeToSave.getNutritionalInfo().value());
                    }
                    if (recipeToSave.getImageUrl().has_value())
                    {
                        recipeToAdd.setImageUrl(recipeToSave.getImageUrl().value());
                    }
                }
                // If idToUse > 0 but not found, it means we are loading from persistence.
                // We use the idToUse (which is recipeToSave.getRecipeId())
                // and recipeToAdd is already a correct copy.
                m_recipes.addBack(recipeToAdd);
            }

            // Ensure m_nextId is always greater than the ID just processed or any existing max ID
            if (idToUse >= m_nextId)
            {
                m_nextId = idToUse + 1;
            }
            // Additionally, ensure m_nextId is greater than any ID in the list (e.g. after loading)
            for (const auto &r : m_recipes)
            {
                if (r.getRecipeId() >= m_nextId)
                {
                    m_nextId = r.getRecipeId() + 1;
                }
            }

            if (saveAll())
            {
                return idToUse; // Return the ID used for saving/updating
            }
            else
            {
                std::cerr << "Error: saveAll() failed after JsonRecipeRepository::save()." << std::endl;
                return -1; // Indicate failure
            }
        }

        bool JsonRecipeRepository::remove(int recipeId)
        {
            // Find the recipe first
            auto it = m_recipes.begin();
            while (it != m_recipes.end())
            {
                if ((*it).getRecipeId() == recipeId)
                {
                    bool removed = m_recipes.removeValue(*it); // Use removeValue based on operator==
                    if (removed)
                    {
                        return saveAll();
                    }
                    else
                    {
                        std::cerr << "Error: Found recipe ID " << recipeId << " but could not remove from list." << std::endl;
                        return false;
                    }
                }
                ++it;
            }
            return false; // Recipe not found
        }

        int JsonRecipeRepository::getNextId() const
        {
            return m_nextId;
        }

    } // namespace Persistence
} // namespace RecipeApp
// Add new method implementation at the end of the file, before the closing namespace brace.

void RecipeApp::Persistence::JsonRecipeRepository::setNextId(int nextId)
{
    m_nextId = nextId;
}