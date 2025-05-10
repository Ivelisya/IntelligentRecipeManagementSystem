#ifndef RESTAURANT_COMMAND_HANDLER_H
#define RESTAURANT_COMMAND_HANDLER_H

#include "cxxopts.hpp"
#include "logic/restaurant/RestaurantManager.h"
#include "logic/recipe/RecipeManager.h" // Added for RecipeManager access

namespace RecipeApp
{
    namespace CliHandlers
    {
        class RestaurantCommandHandler
        {
        public:
            // Constructor now takes RecipeManager as well
            explicit RestaurantCommandHandler(RecipeApp::RestaurantManager &restaurantManager, RecipeApp::RecipeManager &recipeManager);

            // Command handling methods
            int handleAddRestaurant(const cxxopts::ParseResult& result);
            int handleListRestaurants(const cxxopts::ParseResult& result);
            int handleViewRestaurant(const cxxopts::ParseResult& result);
            int handleUpdateRestaurant(const cxxopts::ParseResult& result);
            int handleDeleteRestaurant(const cxxopts::ParseResult& result);
            int handleManageRestaurantMenu(const cxxopts::ParseResult& result); // New method for F1.1.4
            int handleSearchRestaurantsByName(const cxxopts::ParseResult& result); // New method for F1.1.5 (by name)
            int handleSearchRestaurantsByCuisine(const cxxopts::ParseResult& result); // New method for F1.1.5 (by cuisine)

        private:
            RecipeApp::RestaurantManager &restaurantManager_; // Renamed for clarity
            RecipeApp::RecipeManager &recipeManager_;     // Added RecipeManager member
        };
    } // namespace CliHandlers
} // namespace RecipeApp

#endif // RESTAURANT_COMMAND_HANDLER_H