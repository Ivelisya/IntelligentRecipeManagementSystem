#ifndef RESTAURANT_REPOSITORY_H
#define RESTAURANT_REPOSITORY_H

#include "domain/restaurant/Restaurant.h" // Include the definition of Restaurant
#include <string>
#include <vector>
#include <optional> // Include the definition of std::optional

namespace RecipeApp
{
    // Forward declare Restaurant if needed, though Restaurant.h is included.
    // class Restaurant;

    namespace Domain
    {
        namespace Restaurant
        {
            // Use the fully qualified name from the global RecipeApp namespace
            using ::RecipeApp::Restaurant;

            class RestaurantRepository
            {
            public:
                virtual ~RestaurantRepository() = default;

                /**
                 * @brief Finds a restaurant by its unique ID.
                 * @param restaurantId The ID of the restaurant to find.
                 * @return An optional containing the Restaurant if found, otherwise std::nullopt.
                 */
                virtual std::optional<::RecipeApp::Restaurant> findById(int restaurantId) const = 0;

                /**
                 * @brief Finds restaurants by name.
                 * @param name The name to search for.
                 * @param partialMatch If true, performs a substring match; otherwise, exact match.
                 * @return A vector of matching Restaurants.
                 */
                virtual std::vector<::RecipeApp::Restaurant> findByName(const std::string &name, bool partialMatch = false) const = 0;

                /**
                 * @brief Retrieves all restaurants.
                 * @return A vector containing all Restaurants.
                 */
                virtual std::vector<::RecipeApp::Restaurant> findAll() const = 0;

                /**
                 * @brief Saves a restaurant (creates if ID is invalid/0, updates otherwise).
                 * @param restaurant The restaurant object to save. Its ID is used for updates.
                 * @return The ID of the saved restaurant (new or existing), or -1 on failure.
                 */
                virtual int save(const ::RecipeApp::Restaurant &restaurant) = 0;

                /**
                 * @brief Removes a restaurant by its ID.
                 * @param restaurantId The ID of the restaurant to remove.
                 * @return True if removal was successful, false otherwise.
                 */
                virtual bool remove(int restaurantId) = 0;

                /**
                 * @brief Gets the next available ID for a new restaurant.
                 * @return The next available ID.
                 */
                virtual int getNextId() const = 0;

                /**
                 * @brief Sets the next available ID for a new restaurant (for persistence loading).
                 * @param nextId The next ID to be used.
                 */
                virtual void setNextId(int nextId) = 0;
            }; // Closing brace for class RestaurantRepository

        } // namespace Restaurant
    } // namespace Domain
} // namespace RecipeApp

#endif // RESTAURANT_REPOSITORY_H // Ensure #endif is the very last line