#include "persistence/JsonRecipeRepository.h"
#include "domain/recipe/Recipe.h" // Ensure Recipe class definition is included
#include "json.hpp"               // For nlohmann::json
#include <fstream>
#include <iostream> // Required for std::cout, std::endl
#include <stdexcept>
#include <sstream>    // For string stream helpers
#include <algorithm>  // For std::find_if, std::remove_if
#include <filesystem> // For path operations and directory creation

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
        JsonRecipeRepository::JsonRecipeRepository(const std::filesystem::path &baseDirectory, const std::string &fileName)
            : m_nextId(1)
        {
            // Ensure the base directory exists
            if (!std::filesystem::exists(baseDirectory))
            {
                try
                {
                    if (std::filesystem::create_directories(baseDirectory))
                    {
                        // Optional: Log directory creation
                        // std::cout << "Info: Created base directory: " << baseDirectory.string() << std::endl;
                    }
                }
                catch (const std::filesystem::filesystem_error &e)
                {
                    // If base directory creation fails, this is a more critical error
                    // than the previous "data" subdir logic, as the path is explicitly provided.
                    std::cerr << "Critical Error: Could not create base directory '" << baseDirectory.string() << "': " << e.what() << std::endl;
                    // Throw an exception or handle this critical failure appropriately.
                    // For now, we'll let it proceed and fail on file operations, but a throw is better.
                    // throw std::runtime_error("Failed to initialize repository: base directory creation failed.");
                    // Fallback to current directory if absolutely necessary, though not ideal for explicit paths.
                    // For robustness in testing or specific scenarios, one might still want a fallback.
                    // However, for production, an explicit path failing to be created is usually a setup error.
                    m_filePath = fileName; // Fallback to current directory with just filename
                    std::cerr << "Warning: Using fallback file path in current directory: " << m_filePath << std::endl;
                    return;
                }
            }

            m_filePath = (baseDirectory / fileName).string();
            // Optional: Log the final path being used
            // std::cout << "Info: Using data file: " << m_filePath << std::endl;
        }

        // --- Load/Save Operations ---

        bool JsonRecipeRepository::load()
        {
            std::ifstream file(m_filePath);
            if (!file.is_open())
            {
                std::cerr << "Warning: Could not open recipe data file for reading: " << m_filePath << ". Starting with an empty recipe list." << std::endl;
                m_recipes.clear();
                m_nextId = 1;
                return true;
            }

            try
            {
                json data = json::parse(file);
                file.close();

                m_recipes.clear();
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
                                m_recipes.push_back(recipe);
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
                m_recipes.clear();
                m_nextId = 1;
                return false;
            }
            catch (const std::exception &e) // This will now primarily catch issues outside the recipe iteration loop
            {
                std::cerr << "Error loading recipe data (outer scope): " << e.what() << std::endl;
                m_recipes.clear();
                m_nextId = 1;
                return false;
            }
        }

        bool JsonRecipeRepository::saveAll()
        {
            json dataDoc; // Changed variable name to avoid conflict with json type alias
            json recipesJsonArray = json::array();

            for (const auto &recipe : m_recipes)
            {
                recipesJsonArray.push_back(json(recipe)); // Assumes Recipe has a to_json or is nlohmann::json compatible
            }
            dataDoc["recipes"] = recipesJsonArray;

            std::filesystem::path filePathObj(m_filePath);
            // Create a temporary file path, e.g., recipes.json.tmp
            std::filesystem::path tempFilePathObj = filePathObj;
            tempFilePathObj += ".tmp";

            // Ensure the target directory exists (constructor should handle the 'data' subdir creation)
            // but good to double check, especially if m_filePath could be set differently.
            if (!filePathObj.parent_path().empty() && !std::filesystem::exists(filePathObj.parent_path()))
            {
                try
                {
                    std::filesystem::create_directories(filePathObj.parent_path());
                }
                catch (const std::filesystem::filesystem_error &e)
                {
                    std::cerr << "Error: Could not create directory for saving file "
                              << filePathObj.parent_path().string() << ": " << e.what() << std::endl;
                    return false; // Cannot save if directory cannot be created
                }
            }

            // 1. Write to temporary file
            std::ofstream tempFile(tempFilePathObj.string(), std::ios::out | std::ios::trunc);
            if (!tempFile.is_open())
            {
                std::cerr << "Error: Could not open temporary recipe data file for writing: " << tempFilePathObj.string() << std::endl;
                return false;
            }

            try
            {
                tempFile << dataDoc.dump(2); // Pretty print with an indent of 2 spaces
                tempFile.close();            // Close immediately after writing to flush buffers

                if (tempFile.fail())
                { // Check for write errors after closing
                    std::cerr << "Error: Failed to write all data to temporary file: " << tempFilePathObj.string() << std::endl;
                    std::filesystem::remove(tempFilePathObj); // Attempt to clean up
                    return false;
                }
            }
            catch (const std::exception &e) // Catches exceptions from data.dump() or file operations
            {
                std::cerr << "Error serializing recipe data to temporary file '" << tempFilePathObj.string() << "': " << e.what() << std::endl;
                if (tempFile.is_open())
                    tempFile.close();                     // Ensure file is closed
                std::filesystem::remove(tempFilePathObj); // Attempt to clean up
                return false;
            }

            // 2. Replace original file with temporary file
            try
            {
                // std::filesystem::rename should atomically replace the destination if it exists on most modern systems.
                std::filesystem::rename(tempFilePathObj, filePathObj);
                return true;
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                std::cerr << "Error: Failed to replace original file '" << filePathObj.string()
                          << "' with temporary file '" << tempFilePathObj.string() << "': " << e.what() << std::endl;
                // Attempt to clean up the temporary file if it still exists
                if (std::filesystem::exists(tempFilePathObj))
                {
                    std::error_code ec; // To check remove errors without throwing
                    std::filesystem::remove(tempFilePathObj, ec);
                    if (ec)
                    {
                        std::cerr << "Warning: Could not remove temporary file '" << tempFilePathObj.string() << "' after rename failed: " << ec.message() << std::endl;
                    }
                }
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
        std::vector<Recipe> JsonRecipeRepository::findByName(const std::string &name, bool partialMatch) const
        {
            std::cout << "[DEBUG] findByName called with: " << name << ", partialMatch: " << partialMatch << std::endl; // DEBUG
            std::vector<Recipe> results;

            // 转换查询字符串为小写，实现不区分大小写的搜索
            std::string lowerName = name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

            std::cout << "[DEBUG] m_recipes size: " << m_recipes.size() << std::endl; // DEBUG
            int count = 0;                                                            // DEBUG
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
                        results.push_back(recipe);
                    }
                }
                else
                {
                    if (lowerCurrentName == lowerName)
                    {
                        results.push_back(recipe);
                    }
                }
            }
            std::cout << "[DEBUG] findByName finished iteration. Results size: " << results.size() << std::endl; // DEBUG
            return results;
        }

        std::vector<Recipe> JsonRecipeRepository::findAll() const
        {
            std::vector<Recipe> allRecipes;
            allRecipes.reserve(m_recipes.size()); // Optional: pre-allocate memory
            for (const auto &recipe : m_recipes)
            {
                allRecipes.push_back(recipe); // Add a copy of each recipe
            }
            return allRecipes;
        }

        int JsonRecipeRepository::save(const Recipe &recipeToSave)
        {
            int idToUse = recipeToSave.getRecipeId();
            bool isUpdateOperation = false;
            std::optional<Recipe> originalRecipeOpt = std::nullopt;

            // Check if it's an update of an existing recipe
            if (idToUse > 0)
            {
                auto it = std::find_if(m_recipes.begin(), m_recipes.end(),
                                       [idToUse](const Recipe &r)
                                       { return r.getRecipeId() == idToUse; });
                if (it != m_recipes.end())
                {
                    originalRecipeOpt = *it; // Save original for rollback
                    *it = recipeToSave;      // Update in memory
                    isUpdateOperation = true;
                }
            }

            // If not an update, it's an add operation
            if (!isUpdateOperation)
            {
                Recipe recipeToAdd = recipeToSave; // Start with a copy
                int assignedIdForAdd = idToUse;

                if (assignedIdForAdd <= 0)
                { // Needs a new ID
                    assignedIdForAdd = m_nextId;
                }

                // Ensure the recipe object to be added has the correct ID
                recipeToAdd = Recipe(assignedIdForAdd,
                                     recipeToSave.getName(),
                                     recipeToSave.getIngredients(),
                                     recipeToSave.getSteps(),
                                     recipeToSave.getCookingTime(),
                                     recipeToSave.getDifficulty(),
                                     recipeToSave.getCuisine(),
                                     recipeToSave.getTags());
                if (recipeToSave.getNutritionalInfo().has_value())
                    recipeToAdd.setNutritionalInfo(recipeToSave.getNutritionalInfo().value());
                if (recipeToSave.getImageUrl().has_value())
                    recipeToAdd.setImageUrl(recipeToSave.getImageUrl().value());

                m_recipes.push_back(recipeToAdd);
                idToUse = assignedIdForAdd; // Update idToUse to reflect the ID actually added
            }

            // Persist changes
            if (saveAll())
            {
                // If save was successful, update m_nextId
                if (!isUpdateOperation && idToUse == m_nextId)
                { // If a new ID was assigned using m_nextId
                    m_nextId++;
                }
                else
                { // For updates or adds with pre-assigned ID, ensure m_nextId is greater than any existing ID
                    int maxIdInList = 0;
                    for (const auto &r : m_recipes)
                    {
                        if (r.getRecipeId() > maxIdInList)
                        {
                            maxIdInList = r.getRecipeId();
                        }
                    }
                    m_nextId = maxIdInList + 1;
                }
                return idToUse;
            }
            else
            {
                // Persistence failed, roll back memory changes
                std::cerr << "Error: saveAll() failed. Rolling back memory changes for ID: " << idToUse << std::endl;
                if (isUpdateOperation && originalRecipeOpt.has_value())
                {
                    // Rollback update
                    for (auto it = m_recipes.begin(); it != m_recipes.end(); ++it)
                    {
                        if ((*it).getRecipeId() == idToUse)
                        {
                            *it = originalRecipeOpt.value();
                            break;
                        }
                    }
                }
                else if (!isUpdateOperation)
                { // Was an add operation
                    // Rollback add: remove the recipe with idToUse
                    m_recipes.erase(std::remove_if(m_recipes.begin(), m_recipes.end(),
                                                   [idToUse](const Recipe &r)
                                                   { return r.getRecipeId() == idToUse; }),
                                    m_recipes.end());
                    // After removing, m_nextId might need recalculation if the failed add was the highest.
                    // Or, if it was a new ID from m_nextId, it should be safe to just let it be,
                    // as the next attempt to get m_nextId would yield the same value.
                    // For simplicity, we can recalculate m_nextId to be safe.
                    int maxIdAfterRollback = 0;
                    for (const auto &r_rb : m_recipes)
                    {
                        if (r_rb.getRecipeId() > maxIdAfterRollback)
                        {
                            maxIdAfterRollback = r_rb.getRecipeId();
                        }
                    }
                    m_nextId = maxIdAfterRollback + 1;
                }
                return -1; // Indicate failure
            }
        }

        bool JsonRecipeRepository::remove(int recipeId)
        {
            auto initial_size = m_recipes.size();
            m_recipes.erase(
                std::remove_if(m_recipes.begin(), m_recipes.end(),
                               [recipeId](const Recipe &recipe)
                               {
                                   return recipe.getRecipeId() == recipeId;
                               }),
                m_recipes.end());

            if (m_recipes.size() < initial_size) // If an element was removed
            {
                return saveAll();
            }
            // If no element was removed, it means the recipeId was not found.
            // Optionally, log this durumu if it's considered an error or unexpected.
            // std::cerr << "Warning: Recipe ID " << recipeId << " not found for removal." << std::endl;
            return false;
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