#ifndef RESTAURANT_H
#define RESTAURANT_H

#include <algorithm>  // For std::remove and std::find
#include <iostream>   // For displayRestaurantDetails
#include <optional>   // For optional fields in JSON
#include <string>
#include <vector>

#include "json.hpp"  // For JSON serialization

// Convenience alias
using json = nlohmann::json;

namespace RecipeApp {

class RestaurantBuilder;  // Forward declaration

class Restaurant {
    friend class RestaurantBuilder;

   private:
    int restaurantId;
    std::string name;
    std::string address;
    std::string contact;
    std::string openingHours;
    std::vector<int> featuredRecipeIds;

    // Private constructor for Builder
    Restaurant(int id, std::string name, std::string address,
               std::string contact, std::string openingHours,
               std::vector<int> featuredRecipeIds)
        : restaurantId(id),
          name(std::move(name)),
          address(std::move(address)),
          contact(std::move(contact)),
          openingHours(std::move(openingHours)),
          featuredRecipeIds(std::move(featuredRecipeIds)) {
        if (this->name.empty())
            throw std::invalid_argument("Restaurant name cannot be empty.");
        // Address and contact can be optional depending on requirements, for
        // now let's enforce them.
        if (this->address.empty())
            throw std::invalid_argument("Restaurant address cannot be empty.");
        if (this->contact.empty())
            throw std::invalid_argument("Restaurant contact cannot be empty.");
    }

   public:
    static RestaurantBuilder builder(int id, const std::string &name);

    // Default destructor
    ~Restaurant() = default;

    // Getter methods
    int getRestaurantId() const { return restaurantId; }
    int getId() const {
        return getRestaurantId();
    }  // Added for JsonRepositoryBase compatibility
    const std::string &getName() const { return name; }
    const std::string &getAddress() const { return address; }
    const std::string &getContact() const { return contact; }
    const std::string &getOpeningHours() const { return openingHours; }
    const std::vector<int> &getFeaturedRecipeIds() const {
        return featuredRecipeIds;
    }

    // Setter methods
    void setName(const std::string &newName) {
        if (newName.empty())
            throw std::invalid_argument("Restaurant name cannot be empty.");
        name = newName;
    }
    void setAddress(const std::string &newAddress) {
        if (newAddress.empty())
            throw std::invalid_argument("Restaurant address cannot be empty.");
        address = newAddress;
    }
    void setContact(const std::string &newContact) {
        if (newContact.empty())
            throw std::invalid_argument("Restaurant contact cannot be empty.");
        contact = newContact;
    }
    void setOpeningHours(const std::string &newOpeningHours) {
        // Opening hours can be an empty string if not specified
        openingHours = newOpeningHours;
    }
    // Setter for featuredRecipeIds might be useful if direct manipulation is
    // needed post-construction
    void setFeaturedRecipeIds(const std::vector<int> &ids) {
        featuredRecipeIds = ids;
    }

    // Methods for managing featured recipes
    void addFeaturedRecipe(int recipeId);
    void removeFeaturedRecipe(int recipeId);

    // Display method
    void displayRestaurantDetails() const;

    // Overload operator== for comparisons
    bool operator==(const Restaurant &other) const {
        return this->restaurantId == other.restaurantId;
    }
};

class RestaurantBuilder {
   private:
    int m_id;
    std::string m_name;
    std::string m_address;
    std::string m_contact;
    std::string m_openingHours;            // Optional
    std::vector<int> m_featuredRecipeIds;  // Optional

   public:
    RestaurantBuilder(int id, const std::string &name) : m_id(id) {
        if (name.empty())
            throw std::invalid_argument(
                "Restaurant name cannot be empty for builder.");
        m_name = name;
    }

    RestaurantBuilder &withAddress(const std::string &address) {
        if (address.empty())
            throw std::invalid_argument("Restaurant address cannot be empty.");
        m_address = address;
        return *this;
    }

    RestaurantBuilder &withContact(const std::string &contact) {
        if (contact.empty())
            throw std::invalid_argument("Restaurant contact cannot be empty.");
        m_contact = contact;
        return *this;
    }

    RestaurantBuilder &withOpeningHours(const std::string &openingHours) {
        m_openingHours = openingHours;  // Can be empty
        return *this;
    }

    RestaurantBuilder &withFeaturedRecipeIds(const std::vector<int> &ids) {
        m_featuredRecipeIds = ids;
        return *this;
    }

    Restaurant build() const {
        // Ensure required fields are set (name is set in constructor)
        if (m_address.empty())
            throw std::runtime_error(
                "Address must be set to build Restaurant.");
        if (m_contact.empty())
            throw std::runtime_error(
                "Contact must be set to build Restaurant.");
        // ID is also required, set in constructor.

        return Restaurant(m_id, m_name, m_address, m_contact, m_openingHours,
                          m_featuredRecipeIds);
    }
};

// Forward declaration for ADL
void to_json(json &j, const Restaurant &r);

}  // namespace RecipeApp

// --- nlohmann::json adl_serializer specialization for RecipeApp::Restaurant
// ---
namespace nlohmann {
template <>
struct adl_serializer<RecipeApp::Restaurant> {
    // Convert Restaurant to JSON
    static void to_json(json &j, const RecipeApp::Restaurant &r) {
        // Call the free function in RecipeApp namespace
        RecipeApp::to_json(j, r);
    }

    // Convert JSON to Restaurant using the builder
    static RecipeApp::Restaurant from_json(const json &j) {
        int id;
        if (j.contains("id") && j.at("id").is_number_integer()) {
            j.at("id").get_to(id);
            if (id <= 0)
                throw std::runtime_error(
                    "Restaurant ID in JSON must be a positive integer.");
        } else {
            throw std::runtime_error(
                "Restaurant ID is missing or not an integer in JSON.");
        }

        std::string name;
        if (j.contains("name") && j.at("name").is_string()) {
            j.at("name").get_to(name);
            if (name.empty())
                throw std::runtime_error(
                    "Restaurant name cannot be empty in JSON.");
        } else {
            throw std::runtime_error(
                "Restaurant name is missing or not a string in JSON.");
        }

        auto builder = RecipeApp::Restaurant::builder(id, name);

        if (j.contains("address") && j.at("address").is_string()) {
            builder.withAddress(j.at("address").get<std::string>());
        } else {
            throw std::runtime_error(
                "Restaurant address is missing or not a string in JSON.");
        }

        if (j.contains("contact") && j.at("contact").is_string()) {
            builder.withContact(j.at("contact").get<std::string>());
        } else {
            throw std::runtime_error(
                "Restaurant contact is missing or not a string in JSON.");
        }

        if (j.contains("openingHours") && j.at("openingHours").is_string()) {
            builder.withOpeningHours(j.at("openingHours").get<std::string>());
        }
        // openingHours is optional, so no throw if missing

        if (j.contains("featuredRecipeIds") &&
            j.at("featuredRecipeIds").is_array()) {
            std::vector<int> ids;
            for (const auto &recipe_id_json : j.at("featuredRecipeIds")) {
                if (recipe_id_json.is_number_integer()) {
                    ids.push_back(recipe_id_json.get<int>());
                }
            }
            builder.withFeaturedRecipeIds(ids);
        }
        return builder.build();
    }
};
}  // namespace nlohmann

#endif  // RESTAURANT_H