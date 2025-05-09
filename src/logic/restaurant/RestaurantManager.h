#ifndef RESTAURANT_MANAGER_H
#define RESTAURANT_MANAGER_H

#include "domain/restaurant/Restaurant.h"
#include "domain/restaurant/RestaurantRepository.h" // Added RestaurantRepository include
#include "logic/recipe/RecipeManager.h"             // Still needed for getFeaturedRecipes
#include "domain/recipe/Recipe.h"                   // Need Recipe definition
#include <string>
#include <vector>
#include <optional> // For findById return type

namespace RecipeApp
{
    // Forward declare if needed
    // class RecipeManager;
    // class Recipe;

    class RestaurantManager
    {
    private:
        Domain::Restaurant::RestaurantRepository &restaurantRepository_; ///< Reference to the restaurant repository
        // Removed internal list and nextId

    public:
        /**
         * @brief Constructor, takes repository reference.
         * @param restaurantRepository Reference to the restaurant repository.
         */
        explicit RestaurantManager(Domain::Restaurant::RestaurantRepository &restaurantRepository);

        ~RestaurantManager() = default;

        /**
         * @brief Add a new restaurant.
         * @param restaurant Restaurant object to add (ID is ignored, repository assigns).
         * @return The new restaurant's ID if successful, -1 otherwise (e.g., name conflict).
         */
        int addRestaurant(const Restaurant &restaurant); // Changed return type to int (new ID)

        /**
         * @brief Find a restaurant by its ID.
         * @param restaurantId The ID to search for.
         * @return std::optional containing the Restaurant if found.
         */
        std::optional<Restaurant> findRestaurantById(int restaurantId) const;

        /**
         * @brief Find restaurants by name.
         * @param name The name to search for.
         * @param partialMatch Whether to perform partial matching (default: false).
         * @return std::vector of matching Restaurants.
         */
        std::vector<Restaurant> findRestaurantByName(const std::string &name, bool partialMatch = false) const;

        /**
         * @brief Get all restaurants.
         * @return std::vector containing all Restaurants.
         */
        std::vector<Restaurant> getAllRestaurants() const;

        /**
         * @brief Update an existing restaurant.
         * @param restaurant The restaurant object with updated details (must have correct ID).
         * @return True if successful, false otherwise (e.g., not found, name conflict).
         */
        bool updateRestaurant(const Restaurant &restaurant);

        /**
         * @brief Delete a restaurant by its ID.
         * @param restaurantId The ID of the restaurant to delete.
         * @return True if successful, false otherwise.
         */
        bool deleteRestaurant(int restaurantId);

        /**
         * @brief Get featured recipes for a specific restaurant.
         * @param restaurantId The ID of the restaurant.
         * @param recipeManager Reference to RecipeManager to fetch recipe details.
         * @return std::vector of featured Recipe objects. Returns empty vector if restaurant not found.
         */
        std::vector<Recipe> getFeaturedRecipes(int restaurantId, const RecipeManager &recipeManager) const; // Changed return type

        /**
         * @brief Gets the next available ID for a new restaurant (for persistence).
         * @return The next available ID.
         */
        int getNextRestaurantId() const;

        /**
         * @brief Adds a restaurant from persistence (bypasses normal add logic).
         * @param restaurant The restaurant object to add.
         */
        void addRestaurantFromPersistence(const Restaurant &restaurant);

        /**
         * @brief Sets the next restaurant ID from persistence.
         * @param nextId The next ID to be used by the repository.
         */
        void setNextRestaurantIdFromPersistence(int nextId);

        // Removed persistence-specific methods: setNextRestaurantId, getNextRestaurantId, addRestaurantDirectly
    };

} // namespace RecipeApp

#endif // RESTAURANT_MANAGER_H