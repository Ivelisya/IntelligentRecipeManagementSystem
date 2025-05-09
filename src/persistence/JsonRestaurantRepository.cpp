#include "persistence/JsonRestaurantRepository.h"

#include <algorithm>  // For std::transform, std::find_if, std::remove_if
#include <iostream>   // For std::cerr, std::cout (debugging or info)
#include <stdexcept>  // For std::runtime_error

#include "domain/restaurant/Restaurant.h"  // For Restaurant::builder, etc.

// json.hpp is included via JsonRepositoryBase.h
// No need for direct include of fstream, filesystem here as base handles it.

namespace RecipeApp {
namespace Persistence {

// using ::RecipeApp::Restaurant; // Already specified in header or via
// ::RecipeApp::Restaurant

JsonRestaurantRepository::JsonRestaurantRepository(
    const std::filesystem::path &baseDirectory, const std::string &fileName)
    : JsonRepositoryBase<Restaurant>(baseDirectory, fileName, "restaurants") {
    // Constructor of JsonRepositoryBase handles m_filePath, m_nextId,
    // m_jsonArrayKey. Load data upon construction.
    if (!this->load()) {
        std::cerr << "JsonRestaurantRepository: Critical error during "
                     "construction. Failed to load data from "
                  << fileName
                  << ". Repository might be in an inconsistent state."
                  << std::endl;
        // Consider throwing an exception here.
    }
}

bool JsonRestaurantRepository::load() {
    return JsonRepositoryBase<Restaurant>::load();
}

std::optional<::RecipeApp::Restaurant> JsonRestaurantRepository::findById(
    int restaurantId) const {
    return this->findByIdInternal(restaurantId);
}

std::vector<::RecipeApp::Restaurant> JsonRestaurantRepository::findAll() const {
    return this->findAllInternal();
}

int JsonRestaurantRepository::save(
    const ::RecipeApp::Restaurant &restaurantToSave) {
    ::RecipeApp::Restaurant restaurantWithCorrectId =
        restaurantToSave;  // Mutable copy
    bool isNewItem = false;

    if (restaurantToSave.getRestaurantId() <= 0) {
        isNewItem = true;
        int newId = this->getNextId();  // Get next available ID from base

        // Reconstruct Restaurant with the new ID using the builder
        // This is necessary because Restaurant's ID is private and set via
        // builder/constructor
        auto builder =
            ::RecipeApp::Restaurant::builder(newId, restaurantToSave.getName())
                .withAddress(restaurantToSave.getAddress())
                .withContact(restaurantToSave.getContact())
                .withOpeningHours(restaurantToSave.getOpeningHours())
                .withFeaturedRecipeIds(restaurantToSave.getFeaturedRecipeIds());
        // No optional fields like nutritionalInfo or imageUrl in Restaurant
        // based on Restaurant.h
        restaurantWithCorrectId = builder.build();
    }
    // For updates, restaurantToSave (now restaurantWithCorrectId) already has
    // the correct ID.

    if (this->updateOrAddItemInMemoryAndPersist(restaurantWithCorrectId,
                                                isNewItem)) {
        if (isNewItem) {
            this->setNextId(restaurantWithCorrectId.getRestaurantId() +
                            1);  // Update m_nextId in base
        }
        this->ensureNextIdIsCorrect();  // Ensure m_nextId is greater than any
                                        // existing ID
        return restaurantWithCorrectId.getRestaurantId();
    }
    return -1;  // Indicate failure
}

bool JsonRestaurantRepository::remove(int restaurantId) {
    if (this->removeItemInMemoryAndPersist(restaurantId)) {
        this->ensureNextIdIsCorrect();
        return true;
    }
    return false;
}

int JsonRestaurantRepository::getNextId() const {
    return JsonRepositoryBase<Restaurant>::getNextId();
}

void JsonRestaurantRepository::setNextId(int nextId) {
    JsonRepositoryBase<Restaurant>::setNextId(nextId);
}

// --- Restaurant-specific finders ---
std::vector<::RecipeApp::Restaurant> JsonRestaurantRepository::findByName(
    const std::string &name, bool partialMatch) const {
    std::vector<::RecipeApp::Restaurant> results;
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    for (const auto &restaurant :
         this->m_items) {  // Use this->m_items from base
        std::string currentRestaurantName = restaurant.getName();
        std::string lowerCurrentName = currentRestaurantName;
        std::transform(lowerCurrentName.begin(), lowerCurrentName.end(),
                       lowerCurrentName.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (partialMatch) {
            if (lowerCurrentName.find(lowerName) != std::string::npos) {
                results.push_back(restaurant);
            }
        } else {
            if (lowerCurrentName == lowerName) {
                results.push_back(restaurant);
            }
        }
    }
    return results;
}

}  // namespace Persistence
}  // namespace RecipeApp