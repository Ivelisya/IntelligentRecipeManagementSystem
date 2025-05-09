#ifndef JSON_RESTAURANT_REPOSITORY_H
#define JSON_RESTAURANT_REPOSITORY_H

#include "domain/restaurant/RestaurantRepository.h"
#include "domain/restaurant/Restaurant.h" // Use correct path
#include "core/CustomLinkedList.h"        // Keep if needed for internal storage, though vector might be better
#include "json.hpp"                       // nlohmann::json
#include <string>
#include <vector>
#include <optional>
#include <fstream>

namespace RecipeApp
{
    namespace Persistence
    {
        using json = nlohmann::json;
        // Use the fully qualified name or a using declaration for Restaurant from the global namespace
        using Domain::Restaurant::RestaurantRepository; // Use interface from Domain::Restaurant namespace
        using ::RecipeApp::Restaurant;

        class JsonRestaurantRepository : public RestaurantRepository
        {
        public:
            explicit JsonRestaurantRepository(const std::string &filePath);

            // Load/Save operations
            bool load();
            bool saveAll(); // Helper to save the entire list to JSON

            // Implementation of RestaurantRepository interface
            std::optional<::RecipeApp::Restaurant> findById(int restaurantId) const override;
            std::vector<::RecipeApp::Restaurant> findByName(const std::string &name, bool partialMatch = false) const override;
            std::vector<::RecipeApp::Restaurant> findAll() const override;
            int save(const ::RecipeApp::Restaurant &restaurant) override; // Returns ID or -1
            bool remove(int restaurantId) override;
            int getNextId() const override; // Added override, uncommented
            void setNextId(int nextId) override;

        private:
            std::string m_filePath;
            // Consider using std::vector instead of CustomLinkedList for easier management
            CustomDataStructures::CustomLinkedList<::RecipeApp::Restaurant> m_restaurants;
            int m_nextId = 1; // Simple counter for new IDs

            // JSON serialization/deserialization helpers specific to Restaurant
            json restaurantToJson(const ::RecipeApp::Restaurant &restaurant) const;
            ::RecipeApp::Restaurant jsonToRestaurant(const json &j) const;

            // Helpers for complex types like featuredRecipeIds
            std::string serializeFeaturedRecipeIds(const std::vector<int> &ids) const;
            std::vector<int> deserializeFeaturedRecipeIds(const std::string &s) const;
        };

    } // namespace Persistence
} // namespace RecipeApp

#endif // JSON_RESTAURANT_REPOSITORY_H