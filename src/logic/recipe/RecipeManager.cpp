#include "RecipeManager.h"

#include <algorithm>  // For std::all_of, std::any_of, std::transform
#include <iostream>   // For std::cerr in constructor fallback (will be removed later if possible)
#include <locale>     // For std::tolower
#include "spdlog/spdlog.h"
#include "common/exceptions/ValidationException.h"
#include "common/exceptions/PersistenceException.h"
// Note: Cli::InvalidRecipeDataException and Cli::PersistenceException seem to be ad-hoc or undefined.
// They will be fully replaced by the new exception types.
#include <optional>
#include <set>  // Required for index value type
#include <vector>

#include "../../cli/ExitCodes.h"         // Include ExitCodes for exceptions
#include "../../domain/recipe/Recipe.h"  // Ensure Recipe is fully defined
#include "../../domain/recipe/RecipeRepository.h"

// Using declarations for convenience
using RecipeApp::Recipe;
using RecipeApp::Domain::Recipe::RecipeRepository;

namespace RecipeApp {

RecipeManager::RecipeManager(RecipeRepository &recipeRepository)
    : recipeRepository_(recipeRepository) {
    buildInitialIndexes();  // Build indexes on construction
}

// Helper function to normalize strings for indexing (e.g., to lowercase)
std::string RecipeManager::normalizeString(const std::string &str) const {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return lower_str;
}

void RecipeManager::buildInitialIndexes() {
    m_nameIndex.clear();
    m_ingredientIndex.clear();
    m_tagIndex.clear();

    std::vector<Recipe> allRecipes = recipeRepository_.findAll();
    for (const auto &recipe : allRecipes) {
        addRecipeToIndex(recipe);
    }
}

void RecipeManager::addRecipeToIndex(const Recipe &recipe) {
    int recipeId = recipe.getRecipeId();

    // Index by name
    m_nameIndex[normalizeString(recipe.getName())].insert(recipeId);

    // Index by ingredients
    for (const auto &ingredient : recipe.getIngredients()) {
        m_ingredientIndex[normalizeString(ingredient.name)].insert(recipeId);
    }

    // Index by tags
    for (const auto &tag : recipe.getTags()) {
        m_tagIndex[normalizeString(tag)].insert(recipeId);
    }
}

void RecipeManager::removeRecipeFromIndex(const Recipe &recipe) {
    int recipeId = recipe.getRecipeId();

    // Remove from name index
    auto nameIt = m_nameIndex.find(normalizeString(recipe.getName()));
    if (nameIt != m_nameIndex.end()) {
        nameIt->second.erase(recipeId);
        if (nameIt->second.empty()) {
            m_nameIndex.erase(nameIt);
        }
    }

    // Remove from ingredient index
    for (const auto &ingredient : recipe.getIngredients()) {
        auto ingIt = m_ingredientIndex.find(normalizeString(ingredient.name));
        if (ingIt != m_ingredientIndex.end()) {
            ingIt->second.erase(recipeId);
            if (ingIt->second.empty()) {
                m_ingredientIndex.erase(ingIt);
            }
        }
    }

    // Remove from tag index
    for (const auto &tag : recipe.getTags()) {
        auto tagIt = m_tagIndex.find(normalizeString(tag));
        if (tagIt != m_tagIndex.end()) {
            tagIt->second.erase(recipeId);
            if (tagIt->second.empty()) {
                m_tagIndex.erase(tagIt);
            }
        }
    }
}

void RecipeManager::updateRecipeInIndex(const Recipe &oldRecipe,
                                        const Recipe &newRecipe) {
    removeRecipeFromIndex(oldRecipe);
    addRecipeToIndex(newRecipe);
}

int RecipeManager::addRecipe(const Recipe &recipe_param) {
    spdlog::info("尝试添加菜谱: {}", recipe_param.getName());

    // 1. Check for name conflict using the m_nameIndex
    std::string normalized_name = normalizeString(recipe_param.getName());
    if (m_nameIndex.count(normalized_name) && !m_nameIndex.at(normalized_name).empty()) {
        spdlog::warn("尝试添加的菜谱 '{}' (规范化名称: '{}') 与现有菜谱名称冲突。", recipe_param.getName(), normalized_name);
        throw RecipeApp::Common::Exceptions::ValidationException(
            "菜谱名称 '" + recipe_param.getName() + "' 已存在。");
        // return -1; // Unreachable due to throw
    }

    // 2. Create a new Recipe object using the builder
    auto builder = RecipeApp::Recipe::builder(0, recipe_param.getName())
                       .withIngredients(recipe_param.getIngredients())
                       .withSteps(recipe_param.getSteps())
                       .withCookingTime(recipe_param.getCookingTime())
                       .withDifficulty(recipe_param.getDifficulty())
                       .withTags(recipe_param.getTags());

    if (recipe_param.getNutritionalInfo().has_value()) {
        builder.withNutritionalInfo(recipe_param.getNutritionalInfo().value());
    }
    if (recipe_param.getImageUrl().has_value()) {
        builder.withImageUrl(recipe_param.getImageUrl().value());
    }
    RecipeApp::Recipe new_recipe_to_add = builder.build();

    // 3. Save using the repository and return the assigned ID (or -1 on
    // failure)
    try {
        int newRecipeId = recipeRepository_.save(new_recipe_to_add);
        if (newRecipeId != -1) {
            // Retrieve the recipe that was actually saved (it now has the
            // correct ID) This assumes save() in repository correctly assigns
            // ID to the object it processes or returns it. For simplicity,
            // let's assume new_recipe_to_add is what we need to index if ID is
            // positive. A more robust way would be to fetch the recipe by
            // newRecipeId from repository. For now, we'll use the builder's
            // result if it was a new recipe. If save modifies the passed object
            // with ID, then new_recipe_to_add would have it. Let's refine this:
            // the builder creates a recipe with ID 0. The repository's save
            // method should return the recipe with the *actual* ID. However,
            // the current save signature returns int. We need to fetch the
            // recipe by ID to get the fully formed object for indexing.
            std::optional<Recipe> savedRecipeOpt =
                recipeRepository_.findById(newRecipeId);
            if (savedRecipeOpt.has_value()) {
                addRecipeToIndex(savedRecipeOpt.value());
            } else {
                // This case should ideally not happen if save was successful
                spdlog::error("菜谱已保存，ID: {}, 但无法从仓库检索以更新索引。", newRecipeId);
            }
        }
        spdlog::info("菜谱 '{}' 添加成功，ID: {}", recipe_param.getName(), newRecipeId);
        return newRecipeId;
    } catch (const RecipeApp::Common::Exceptions::PersistenceException& pe) { // Catch our own first
        spdlog::error("保存菜谱 '{}' 时发生持久化错误: {}", recipe_param.getName(), pe.what());
        throw; // Re-throw
    } catch (const std::exception &e) { // Catch other std::exceptions
        spdlog::error("保存菜谱 '{}' 时发生未知错误: {}", recipe_param.getName(), e.what());
        // Wrap generic std::exception into our PersistenceException for consistent API
        throw RecipeApp::Common::Exceptions::PersistenceException(
            "保存菜谱 '" + recipe_param.getName() + "' 失败: " + std::string(e.what()));
    }
}

// findRecipeByName now uses the m_nameIndex
std::vector<Recipe> RecipeManager::findRecipeByName(const std::string &name,
                                                    bool partialMatch) const {
    std::string normalized_query = normalizeString(name);
    std::set<int> matched_ids;

    if (!partialMatch) {
        auto it = m_nameIndex.find(normalized_query);
        if (it != m_nameIndex.end()) {
            matched_ids = it->second;
        }
    } else {
        for (const auto &pair : m_nameIndex) {
            // pair.first is the normalized recipe name from the index
            if (pair.first.find(normalized_query) != std::string::npos) {
                matched_ids.insert(pair.second.begin(), pair.second.end());
            }
        }
    }

    if (matched_ids.empty()) {
        return {};
    }
    std::vector<int> ids_vec(matched_ids.begin(), matched_ids.end());
    return recipeRepository_.findManyByIds(ids_vec);
}

bool RecipeManager::deleteRecipe(int recipeId) {
    std::optional<Recipe> recipeToDeleteOpt =
        recipeRepository_.findById(recipeId);
    if (recipeToDeleteOpt.has_value()) {
        if (recipeRepository_.remove(recipeId)) {
            removeRecipeFromIndex(recipeToDeleteOpt.value());
            return true;
        }
    }
    return false;
}

bool RecipeManager::updateRecipe(const Recipe &updated_recipe_param) {
    // 1. Check if the recipe exists
    std::optional<Recipe> existingRecipeOpt =
        recipeRepository_.findById(updated_recipe_param.getRecipeId());
    if (!existingRecipeOpt.has_value()) {
        spdlog::warn("尝试更新的菜谱 ID: {} 未找到。", updated_recipe_param.getRecipeId());
        return false;  // Recipe not found
    }
    Recipe existingRecipe = existingRecipeOpt.value();

    // 2. Check for name conflict if the name has changed
    if (normalizeString(existingRecipe.getName()) != normalizeString(updated_recipe_param.getName())) {
        std::string normalized_new_name = normalizeString(updated_recipe_param.getName());
        if (m_nameIndex.count(normalized_new_name)) {
            // Check if the conflicting IDs are different from the current recipe's ID
            for (int conflicting_id : m_nameIndex.at(normalized_new_name)) {
                if (conflicting_id != updated_recipe_param.getRecipeId()) {
                    spdlog::warn("更新菜谱 ID: {} 时，新名称 '{}' (规范化: '{}') 与现有菜谱 ID: {} 冲突。",
                                 updated_recipe_param.getRecipeId(), updated_recipe_param.getName(), normalized_new_name, conflicting_id);
                    // Consider throwing ValidationException here if preferred for consistency
                    // throw RecipeApp::Common::Exceptions::ValidationException(
                    //     "更新后的菜谱名称 '" + updated_recipe_param.getName() + "' 与其他菜谱冲突。");
                    return false; // Name conflict with another recipe
                }
            }
        }
    }

    // 3. Save the updated recipe using the repository
    // The save method handles both create and update logic based on ID.
    if (recipeRepository_.save(updated_recipe_param) != -1) {
        // existingRecipe is the state *before* the update.
        // updated_recipe_param is the state *after* the update.
        updateRecipeInIndex(existingRecipe, updated_recipe_param);
        return true;
    }
    return false;
}

std::vector<Recipe> RecipeManager::getAllRecipes() const {
    return recipeRepository_.findAll();
}

// Helper function for set intersection
template <typename T>
std::set<T> intersect_sets(const std::vector<std::set<T>> &sets_to_intersect) {
    if (sets_to_intersect.empty()) {
        return {};
    }
    std::set<T> result_set = sets_to_intersect[0];
    for (size_t i = 1; i < sets_to_intersect.size(); ++i) {
        std::set<T> temp_set;
        std::set_intersection(result_set.begin(), result_set.end(),
                              sets_to_intersect[i].begin(),
                              sets_to_intersect[i].end(),
                              std::inserter(temp_set, temp_set.begin()));
        result_set = temp_set;
        if (result_set.empty()) {  // Optimization: if intersection is empty, no
                                   // need to continue
            break;
        }
    }
    return result_set;
}

// Helper function for set union
template <typename T>
std::set<T> union_sets(const std::vector<std::set<T>> &sets_to_union) {
    std::set<T> result_set;
    for (const auto &s : sets_to_union) {
        result_set.insert(s.begin(), s.end());
    }
    return result_set;
}

std::vector<Recipe> RecipeManager::findRecipesByIngredients(
    const std::vector<std::string> &ingredientsToFind, bool matchAll) const {
    if (ingredientsToFind.empty()) {
        return {};
    }

    std::vector<std::set<int>> id_sets;
    for (const auto &ing_name : ingredientsToFind) {
        std::string normalized_ing = normalizeString(ing_name);
        auto it = m_ingredientIndex.find(normalized_ing);
        if (it != m_ingredientIndex.end() && !it->second.empty()) {
            id_sets.push_back(it->second);
        } else {
            if (matchAll) {
                // If one ingredient is not found in any recipe, and we need to match all,
                // then no recipe can possibly match.
                return {};
            }
            // If not matchAll, we simply ignore this ingredient as it yields no recipes.
            // Or, if it's the very first ingredient and matchAll is false, and it's not found,
            // we still need to continue to see if other ingredients yield results.
            // If id_sets remains empty after checking all ingredients (for matchAll=false),
            // then the result is empty.
        }
    }

    if (id_sets.empty()) {  // No recipes found for any of the specified ingredients
        return {};
    }

    std::set<int> final_ids;
    if (matchAll) {
        final_ids = intersect_sets(id_sets);
    } else {
        final_ids = union_sets(id_sets);
    }

    if (final_ids.empty()) {
        return {};
    }
    std::vector<int> ids_vec(final_ids.begin(), final_ids.end());
    return recipeRepository_.findManyByIds(ids_vec);
}

std::optional<Recipe> RecipeManager::findRecipeById(int recipeId) const {
    return recipeRepository_.findById(recipeId);
}

// addRecipeDirectly and setNextRecipeId are removed as they are repository
// concerns.

void RecipeManager::addRecipeFromPersistence(const Recipe &recipe) {
    recipeRepository_.save(
        recipe);  // Assuming save handles both new and existing if ID is set
}

void RecipeManager::setNextRecipeIdFromPersistence(int nextId) {
    recipeRepository_.setNextId(nextId);
}

// --- New Tag-related Method Implementations ---

std::vector<Recipe> RecipeManager::findRecipesByTag(
    const std::string &tag) const {
    if (tag.empty()) {
        return {};
    }
    std::string normalized_tag = normalizeString(tag);
    auto it = m_tagIndex.find(normalized_tag);
    if (it != m_tagIndex.end() && !it->second.empty()) {
        std::vector<int> ids_vec(it->second.begin(), it->second.end());
        return recipeRepository_.findManyByIds(ids_vec);
    }
    return {};
}

std::vector<Recipe> RecipeManager::findRecipesByTags(
    const std::vector<std::string> &tagsToFind, bool matchAll) const {
    if (tagsToFind.empty()) {
        return {};
    }

    std::vector<std::set<int>> id_sets;
    // bool first_set_added = false; // Unused variable

    for (const auto &tag_name : tagsToFind) {
        std::string normalized_tag = normalizeString(tag_name);
        auto it = m_tagIndex.find(normalized_tag);

        if (it != m_tagIndex.end() && !it->second.empty()) {
            id_sets.push_back(it->second);
            // first_set_added = true; // Unused variable
        } else {
            if (matchAll) {
                // If a tag is not found in any recipe, and we need to match
                // all, then no recipe can match.
                return {};
            }
            // If not matchAll, we simply ignore this tag as it yields no
            // recipes. Or, if it's the very first tag and matchAll is false,
            // and it's not found, we still need to continue to see if other
            // tags yield results. If id_sets remains empty after checking all
            // tags (for matchAll=false), then result is empty.
        }
    }

    if (id_sets.empty()) {  // No recipes found for any of the specified tags
        return {};
    }

    std::set<int> final_ids;
    if (matchAll) {
        final_ids = intersect_sets(id_sets);
    } else {
        final_ids = union_sets(id_sets);
    }

    if (final_ids.empty()) {
        return {};
    }
    std::vector<int> ids_vec(final_ids.begin(), final_ids.end());
    return recipeRepository_.findManyByIds(ids_vec);
}

std::vector<Recipe> RecipeManager::findRecipesByIds(
    const std::vector<int> &ids) const {
    if (ids.empty()) {
        return {};
    }
    // This method should use the recipeRepository_ member to call the
    // repository's method. The repository's findManyByIds is what actually
    // fetches from persistence.
    return recipeRepository_.findManyByIds(ids);
}

}  // namespace RecipeApp

// Add new method implementations at the end of the file, before the closing
// namespace brace.
// The findRecipesByIds was moved inside the namespace.