#include "RestaurantCommandHandler.h"

namespace RecipeApp
{
    namespace CliHandlers
    {
        RestaurantCommandHandler::RestaurantCommandHandler(RecipeApp::RestaurantManager &restaurantManager)
            : restaurantManager(restaurantManager) {}
    } // namespace CliHandlers
} // namespace RecipeApp