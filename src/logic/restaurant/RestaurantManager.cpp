#include "logic/restaurant/RestaurantManager.h"

#include <algorithm>  // For std::transform, std::find_if
#include <cctype>     // For ::tolower
#include <iostream>   // For potential logging
#include <optional>
#include <vector>

#include "domain/recipe/Recipe.h"          // For getFeaturedRecipes
#include "domain/restaurant/Restaurant.h"  // Ensure Restaurant is fully defined
#include "domain/restaurant/RestaurantRepository.h"
#include "logic/recipe/RecipeManager.h"  // For getFeaturedRecipes

// Using declarations for convenience
using RecipeApp::Recipe;  // Need Recipe type
using RecipeApp::Restaurant;
using RecipeApp::Domain::Restaurant::RestaurantRepository;

namespace RecipeApp {

RestaurantManager::RestaurantManager(RestaurantRepository &restaurantRepository)
    : restaurantRepository_(restaurantRepository) {}

int RestaurantManager::addRestaurant(const Restaurant &restaurant_param) {
    // 1. Check for name uniqueness using the repository
    if (!restaurantRepository_.findByName(restaurant_param.getName(), false)
             .empty()) {
        // std::cerr << "RestaurantManager: Restaurant name '" <<
        // restaurant_param.getName() << "' already exists." << std::endl;
        return -1;  // Name conflict
    }

    // 2. Create a new Restaurant object using the builder
    Restaurant newRestaurantToAdd =
        Restaurant::builder(0, restaurant_param.getName())
            .withAddress(restaurant_param.getAddress())
            .withContact(restaurant_param.getContact())
            .withOpeningHours(restaurant_param.getOpeningHours())
            .withFeaturedRecipeIds(restaurant_param.getFeaturedRecipeIds())
            .build();

    // 3. Save using the repository and return the assigned ID (or -1 on
    // failure)
    return restaurantRepository_.save(newRestaurantToAdd);
}

std::optional<Restaurant> RestaurantManager::findRestaurantById(
    int restaurantId) const {
    return restaurantRepository_.findById(restaurantId);
}

std::vector<Restaurant> RestaurantManager::findRestaurantByName(
    const std::string &name, bool partialMatch) const {
    // Basic implementation - delegates directly to repository.
    // Could add caching or additional logic here later if needed.
    // Consider case-insensitivity handling here or in repository.
    return restaurantRepository_.findByName(name, partialMatch);
}

std::vector<Restaurant> RestaurantManager::getAllRestaurants() const {
    return restaurantRepository_.findAll();
}

bool RestaurantManager::updateRestaurant(
    const Restaurant &updated_restaurant_param) {
    // 1. Check if the restaurant exists
    std::optional<Restaurant> existingRestaurantOpt =
        restaurantRepository_.findById(
            updated_restaurant_param.getRestaurantId());
    if (!existingRestaurantOpt.has_value()) {
        // std::cerr << "RestaurantManager: Restaurant to update (ID: " <<
        // updated_restaurant_param.getRestaurantId() << ") not found." <<
        // std::endl;
        return false;  // Restaurant not found
    }
    Restaurant existingRestaurant = existingRestaurantOpt.value();

    // 2. Check for name conflict if the name has changed
    if (existingRestaurant.getName() != updated_restaurant_param.getName()) {
        // Check if the new name exists for any *other* restaurant
        std::vector<Restaurant> potentialConflicts =
            restaurantRepository_.findByName(updated_restaurant_param.getName(),
                                             false);
        for (const auto &conflict : potentialConflicts) {
            if (conflict.getRestaurantId() !=
                updated_restaurant_param.getRestaurantId()) {
                // std::cerr << "RestaurantManager: Updated restaurant name '"
                // << updated_restaurant_param.getName() << "' conflicts with
                // existing restaurant (ID: " << conflict.getRestaurantId() <<
                // ")." << std::endl;
                return false;  // Name conflict with another restaurant
            }
        }
    }

    // 3. Save the updated restaurant using the repository
    return restaurantRepository_.save(updated_restaurant_param) != -1;
}

bool RestaurantManager::deleteRestaurant(int restaurantId) {
    return restaurantRepository_.remove(restaurantId);
}

std::vector<Recipe> RestaurantManager::getFeaturedRecipes(
    int restaurantId, const RecipeManager &recipeManager) const {
    std::vector<Recipe> featuredRecipesResult;
    std::optional<Restaurant> restaurantOpt =
        restaurantRepository_.findById(restaurantId);

    if (restaurantOpt.has_value()) {
        const auto &recipeIds = restaurantOpt->getFeaturedRecipeIds();
        if (!recipeIds.empty()) {
            // Use the new batch find method from RecipeManager
            featuredRecipesResult = recipeManager.findRecipesByIds(recipeIds);
        }
        // The recipeManager.findRecipesByIds will only return recipes that were
        // found. No need to check optional or log warnings here for individual
        // missing recipes, as that logic is (or should be) within
        // findRecipesByIds or its repository call.
    }
    // else {
    //     std::cerr << "Warning: Restaurant with ID " << restaurantId << " not
    //     found when getting featured recipes." << std::endl;
    // }
    return featuredRecipesResult;
}

// Removed persistence-specific methods: setNextRestaurantId,
// getNextRestaurantId, addRestaurantDirectly

}  // namespace RecipeApp
// Add new method implementation at the end of the file, before the closing
// namespace brace.

int RecipeApp::RestaurantManager::getNextRestaurantId() const {
    return restaurantRepository_.getNextId();
}
// Add new method implementations at the end of the file, before the closing
// namespace brace.

void RecipeApp::RestaurantManager::addRestaurantFromPersistence(
    const Restaurant &restaurant) {
    restaurantRepository_.save(restaurant);  // Assuming save handles both new
                                             // and existing if ID is set
}

void RecipeApp::RestaurantManager::setNextRestaurantIdFromPersistence(
    int nextId) {
    restaurantRepository_.setNextId(nextId);
}