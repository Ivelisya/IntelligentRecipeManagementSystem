#ifndef RESTAURANT_COMMAND_HANDLER_H
#define RESTAURANT_COMMAND_HANDLER_H

#include "cxxopts.hpp"
#include "logic/restaurant/RestaurantManager.h"

namespace RecipeApp
{
    namespace CliHandlers
    {
        class RestaurantCommandHandler
        {
        public:
            explicit RestaurantCommandHandler(RecipeApp::RestaurantManager &restaurantManager);

        private:
            RecipeApp::RestaurantManager &restaurantManager;
        };
    } // namespace CliHandlers
} // namespace RecipeApp

#endif // RESTAURANT_COMMAND_HANDLER_H