#include "persistence/JsonRecipeRepository.h"

#include <algorithm>
#include <iostream>  // For std::cerr, std::cout (debugging or info)
#include <stdexcept>  // For std::runtime_error (though not explicitly used in this refactor)

#include "domain/recipe/Recipe.h"  // For Recipe::builder, etc.

// json.hpp is included via JsonRepositoryBase.h
// No need for direct include of fstream, filesystem here as base handles it.

namespace RecipeApp {
namespace Persistence {

// using RecipeApp::Difficulty; // Not directly used in this file's logic after
// refactor
using RecipeApp::Recipe;

JsonRecipeRepository::JsonRecipeRepository(
    const std::filesystem::path &baseDirectory, const std::string &fileName)
    : JsonRepositoryBase<Recipe>(baseDirectory, fileName, "recipes") {
    // Constructor of JsonRepositoryBase handles m_filePath, m_nextId,
    // m_jsonArrayKey. It also attempts to create baseDirectory if it doesn't
    // exist. Load data upon construction.
    if (!this->load()) {  // `this->` is optional for public base methods but
                          // can be explicit
        std::cerr << "JsonRecipeRepository: Critical error during "
                     "construction. Failed to load data from "
                  << fileName
                  << ". Repository might be in an inconsistent state."
                  << std::endl;
        // Depending on application requirements, might throw an exception here.
        // throw std::runtime_error("Failed to load recipe data during
        // repository construction.");
    }
}

bool JsonRecipeRepository::load() {
    // Delegate to base class's public load method.
    // This also ensures m_nextId is updated by the base class.
    return JsonRepositoryBase<Recipe>::load();
}

std::optional<Recipe> JsonRecipeRepository::findById(int recipeId) const {
    return this->findByIdInternal(recipeId);
}

std::vector<Recipe> JsonRecipeRepository::findAll() const {
    return this->findAllInternal();
}

int JsonRecipeRepository::save(const Recipe &recipeToSave) {
    Recipe recipeWithCorrectId = recipeToSave;  // Make a mutable copy
    bool isNewItem = false;

    if (recipeToSave.getRecipeId() <= 0) {
        isNewItem = true;
        int newId = this->getNextId();  // Get next available ID from base

        // Reconstruct Recipe with the new ID using the builder
        auto builder = Recipe::builder(newId, recipeToSave.getName())
                           .withIngredients(recipeToSave.getIngredients())
                           .withSteps(recipeToSave.getSteps())
                           .withCookingTime(recipeToSave.getCookingTime())
                           .withDifficulty(recipeToSave.getDifficulty())
                           .withCuisine(recipeToSave.getCuisine())
                           .withTags(recipeToSave.getTags());
        if (recipeToSave.getNutritionalInfo().has_value()) {
            builder.withNutritionalInfo(
                recipeToSave.getNutritionalInfo().value());
        }
        if (recipeToSave.getImageUrl().has_value()) {
            builder.withImageUrl(recipeToSave.getImageUrl().value());
        }
        recipeWithCorrectId = builder.build();
    }
    // For updates, recipeToSave (now recipeWithCorrectId) already has the
    // correct ID.

    if (this->updateOrAddItemInMemoryAndPersist(recipeWithCorrectId,
                                                isNewItem)) {
        if (isNewItem) {
            // The m_nextId in base was used, now increment it for the *next*
            // new item.
            this->setNextId(recipeWithCorrectId.getId() + 1);
        }
        // Ensure m_nextId is always greater than any existing ID, especially
        // after updates or loading.
        this->ensureNextIdIsCorrect();
        return recipeWithCorrectId.getId();
    }
    return -1;  // Indicate failure
}

bool JsonRecipeRepository::remove(int recipeId) {
    if (this->removeItemInMemoryAndPersist(recipeId)) {
        this->ensureNextIdIsCorrect();  // Recalculate m_nextId in base after
                                        // removal
        return true;
    }
    return false;
}

int JsonRecipeRepository::getNextId() const {
    return JsonRepositoryBase<Recipe>::getNextId();
}

void JsonRecipeRepository::setNextId(int nextId) {
    JsonRepositoryBase<Recipe>::setNextId(nextId);
    // After externally setting an ID (e.g., from persistence loading in
    // manager), it's good practice to ensure it's valid relative to current
    // items. However, load() in base class already does this. If this is called
    // outside of load, then ensureNextIdIsCorrect might be useful. For now,
    // direct set is sufficient as per interface.
}

// --- Recipe-specific finders ---
// These operate on m_items (protected member from JsonRepositoryBase<Recipe>)

std::vector<Recipe> JsonRecipeRepository::findByName(const std::string &name,
                                                     bool partialMatch) const {
    std::vector<Recipe> results;
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    for (const auto &recipe : this->m_items) {
        std::string currentRecipeName = recipe.getName();
        std::string lowerCurrentName = currentRecipeName;
        std::transform(lowerCurrentName.begin(), lowerCurrentName.end(),
                       lowerCurrentName.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (partialMatch) {
            if (lowerCurrentName.find(lowerName) != std::string::npos) {
                results.push_back(recipe);
            }
        } else {
            if (lowerCurrentName == lowerName) {
                results.push_back(recipe);
            }
        }
    }
    return results;
}

std::vector<Recipe> JsonRecipeRepository::findManyByIds(
    const std::vector<int> &ids) const {
    std::vector<Recipe> results;
    if (ids.empty() || this->m_items.empty()) {
        return results;
    }
    results.reserve(ids.size());
    for (const auto &recipe : this->m_items) {
        if (std::find(ids.begin(), ids.end(), recipe.getRecipeId()) !=
            ids.end()) {
            results.push_back(recipe);
        }
    }
    return results;
}

std::vector<Recipe> JsonRecipeRepository::findByCuisine(
    const std::string &cuisineName) const {
    std::vector<Recipe> results;
    if (cuisineName.empty()) {
        return results;
    }
    std::string lowerCuisineName = cuisineName;
    std::transform(lowerCuisineName.begin(), lowerCuisineName.end(),
                   lowerCuisineName.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    for (const auto &recipe : this->m_items) {
        std::string currentRecipeCuisine = recipe.getCuisine();
        std::string lowerCurrentRecipeCuisine = currentRecipeCuisine;
        std::transform(lowerCurrentRecipeCuisine.begin(),
                       lowerCurrentRecipeCuisine.end(),
                       lowerCurrentRecipeCuisine.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (lowerCurrentRecipeCuisine == lowerCuisineName) {
            results.push_back(recipe);
        }
    }
    return results;
}

std::vector<Recipe> JsonRecipeRepository::findByTag(
    const std::string &tagName) const {
    std::vector<Recipe> results;
    if (tagName.empty()) {
        return results;
    }
    std::string lowerTagName = tagName;
    std::transform(lowerTagName.begin(), lowerTagName.end(),
                   lowerTagName.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    for (const auto &recipe : this->m_items) {
        const auto &currentRecipeTags = recipe.getTags();
        for (const auto &tag : currentRecipeTags) {
            std::string lowerCurrentTag = tag;
            std::transform(lowerCurrentTag.begin(), lowerCurrentTag.end(),
                           lowerCurrentTag.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            if (lowerCurrentTag == lowerTagName) {
                results.push_back(recipe);
                break;
            }
        }
    }
    return results;
}

std::vector<Recipe> JsonRecipeRepository::findByIngredients(
    const std::vector<std::string> &ingredientNames, bool matchAll) const {
    std::vector<Recipe> results;
    if (ingredientNames.empty()) {
        return results;
    }

    std::vector<std::string> lowerIngredientNames;
    lowerIngredientNames.reserve(ingredientNames.size());
    for (const auto &name : ingredientNames) {
        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        lowerIngredientNames.push_back(lowerName);
    }

    for (const auto &recipe : this->m_items) {
        const auto &currentRecipeIngredients = recipe.getIngredients();
        std::vector<std::string> currentRecipeIngredientNamesLower;
        currentRecipeIngredientNamesLower.reserve(
            currentRecipeIngredients.size());
        for (const auto &ingredient : currentRecipeIngredients) {
            std::string lowerIngName = ingredient.name;
            std::transform(lowerIngName.begin(), lowerIngName.end(),
                           lowerIngName.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            currentRecipeIngredientNamesLower.push_back(lowerIngName);
        }

        bool recipeMatches = false;
        if (matchAll) {
            recipeMatches = std::all_of(
                lowerIngredientNames.begin(), lowerIngredientNames.end(),
                [&](const std::string &searchIngName) {
                    return std::find(currentRecipeIngredientNamesLower.begin(),
                                     currentRecipeIngredientNamesLower.end(),
                                     searchIngName) !=
                           currentRecipeIngredientNamesLower.end();
                });
        } else {
            recipeMatches = std::any_of(
                lowerIngredientNames.begin(), lowerIngredientNames.end(),
                [&](const std::string &searchIngName) {
                    return std::find(currentRecipeIngredientNamesLower.begin(),
                                     currentRecipeIngredientNamesLower.end(),
                                     searchIngName) !=
                           currentRecipeIngredientNamesLower.end();
                });
        }

        if (recipeMatches) {
            results.push_back(recipe);
        }
    }
    return results;
}

std::vector<Recipe> JsonRecipeRepository::findByTags(
    const std::vector<std::string> &tagNames, bool matchAll) const {
    std::vector<Recipe> results;
    if (tagNames.empty()) {
        return results;
    }

    std::vector<std::string> lowerTagNames;
    lowerTagNames.reserve(tagNames.size());
    for (const auto &name : tagNames) {
        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        lowerTagNames.push_back(lowerName);
    }

    for (const auto &recipe : this->m_items) {
        const auto &currentRecipeTags = recipe.getTags();
        std::vector<std::string> lowerCurrentRecipeTags;
        lowerCurrentRecipeTags.reserve(currentRecipeTags.size());
        for (const auto &tag : currentRecipeTags) {
            std::string lowerTag = tag;
            std::transform(lowerTag.begin(), lowerTag.end(), lowerTag.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            lowerCurrentRecipeTags.push_back(lowerTag);
        }

        bool recipeMatches = false;
        if (matchAll) {
            recipeMatches =
                std::all_of(lowerTagNames.begin(), lowerTagNames.end(),
                            [&](const std::string &searchTagName) {
                                return std::find(lowerCurrentRecipeTags.begin(),
                                                 lowerCurrentRecipeTags.end(),
                                                 searchTagName) !=
                                       lowerCurrentRecipeTags.end();
                            });
        } else {
            recipeMatches =
                std::any_of(lowerTagNames.begin(), lowerTagNames.end(),
                            [&](const std::string &searchTagName) {
                                return std::find(lowerCurrentRecipeTags.begin(),
                                                 lowerCurrentRecipeTags.end(),
                                                 searchTagName) !=
                                       lowerCurrentRecipeTags.end();
                            });
        }

        if (recipeMatches) {
            results.push_back(recipe);
        }
    }
    return results;
}

}  // namespace Persistence
}  // namespace RecipeApp