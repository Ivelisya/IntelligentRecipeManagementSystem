#ifndef RESTAURANT_H
#define RESTAURANT_H

#include <string>
#include <vector>
#include <iostream>  // For displayRestaurantDetails
#include <algorithm> // For std::remove and std::find

namespace RecipeApp
{

    class Restaurant
    {
    private:
        int restaurantId;
        std::string name;
        std::string address;
        std::string contact;
        std::string openingHours;
        std::vector<int> featuredRecipeIds;

    public:
        // Constructor
        Restaurant(int id, const std::string &name, const std::string &address,
                   const std::string &contact, const std::string &openingHours);

        // Default destructor
        ~Restaurant() = default;

        // Getter methods
        int getRestaurantId() const { return restaurantId; }
        const std::string &getName() const { return name; }
        const std::string &getAddress() const { return address; }
        const std::string &getContact() const { return contact; }
        const std::string &getOpeningHours() const { return openingHours; }
        const std::vector<int> &getFeaturedRecipeIds() const { return featuredRecipeIds; }

        // Setter methods
        void setName(const std::string &newName) { name = newName; }
        void setAddress(const std::string &newAddress) { address = newAddress; }
        void setContact(const std::string &newContact) { contact = newContact; }
        void setOpeningHours(const std::string &newOpeningHours) { openingHours = newOpeningHours; }

        // Methods for managing featured recipes
        void addFeaturedRecipe(int recipeId);
        void removeFeaturedRecipe(int recipeId);

        // Display method
        void displayRestaurantDetails() const;

        // Overload operator== for CustomLinkedList operations or other comparisons
        bool operator==(const Restaurant &other) const
        {
            return this->restaurantId == other.restaurantId;
        }
    };

} // namespace RecipeApp

#endif // RESTAURANT_H