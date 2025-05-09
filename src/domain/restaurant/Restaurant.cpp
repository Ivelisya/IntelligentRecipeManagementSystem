#include "Restaurant.h"
#include <algorithm> // Required for std::find and std::remove

namespace RecipeApp
{

    Restaurant::Restaurant(int id, const std::string &name, const std::string &address,
                           const std::string &contact, const std::string &openingHours)
        : restaurantId(id), name(name), address(address), contact(contact), openingHours(openingHours)
    {
        // featuredRecipeIds is initialized as an empty vector by default
    }

    void Restaurant::addFeaturedRecipe(int recipeId)
    {
        // Check if the recipeId already exists to avoid duplicates
        if (std::find(featuredRecipeIds.begin(), featuredRecipeIds.end(), recipeId) == featuredRecipeIds.end())
        {
            featuredRecipeIds.push_back(recipeId);
        }
    }

    void Restaurant::removeFeaturedRecipe(int recipeId)
    {
        // std::remove moves the elements to be removed to the end and returns an iterator to the new end.
        // vector::erase then actually removes the elements from the vector.
        auto it = std::remove(featuredRecipeIds.begin(), featuredRecipeIds.end(), recipeId);
        if (it != featuredRecipeIds.end()) // Check if the element was found and removed
        {
            featuredRecipeIds.erase(it, featuredRecipeIds.end());
        }
    }

    void Restaurant::displayRestaurantDetails() const
    {
        std::cout << "Restaurant ID: " << restaurantId << std::endl;
        std::cout << "Name: " << name << std::endl;
        std::cout << "Address: " << address << std::endl;
        std::cout << "Contact: " << contact << std::endl;
        std::cout << "Opening Hours: " << openingHours << std::endl;
        std::cout << "Featured Recipe IDs: ";
        if (featuredRecipeIds.empty())
        {
            std::cout << "None";
        }
        else
        {
            for (size_t i = 0; i < featuredRecipeIds.size(); ++i)
            {
                std::cout << featuredRecipeIds[i] << (i == featuredRecipeIds.size() - 1 ? "" : ", ");
            }
        }
        std::cout << std::endl;
    }

} // namespace RecipeApp