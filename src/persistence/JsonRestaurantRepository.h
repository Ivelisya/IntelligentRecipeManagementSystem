#ifndef JSON_RESTAURANT_REPOSITORY_H
#define JSON_RESTAURANT_REPOSITORY_H

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "JsonRepositoryBase.h"  // Include the new base class
#include "domain/restaurant/Restaurant.h"
#include "domain/restaurant/RestaurantRepository.h"
// json.hpp is included by JsonRepositoryBase.h

namespace RecipeApp {
namespace Persistence {

// using json = nlohmann::json; // Already in JsonRepositoryBase.h
using Domain::Restaurant::RestaurantRepository;
using ::RecipeApp::Restaurant;  // Keep this for clarity

class JsonRestaurantRepository
    : public JsonRepositoryBase<Restaurant>,
      virtual public RestaurantRepository {  // virtual inheritance
   public:
    explicit JsonRestaurantRepository(
        const std::filesystem::path &baseDirectory,
        const std::string &fileName = "restaurants.json");

    // ~JsonRestaurantRepository() override = default; // If
    // RestaurantRepository has virtual destructor

    // Publicly expose load from base if needed
    bool load();

    // Implementation of RestaurantRepository interface
    std::optional<::RecipeApp::Restaurant> findById(
        int restaurantId) const override;
    std::vector<::RecipeApp::Restaurant> findAll() const override;
    int save(const ::RecipeApp::Restaurant &restaurant) override;
    bool remove(int restaurantId) override;

    int getNextId() const override;
    void setNextId(int nextId) override;

    // Restaurant-specific finders (if any beyond the interface, though
    // findByName is in interface)
    std::vector<::RecipeApp::Restaurant> findByName(
        const std::string &name, bool partialMatch = false) const override;

   private:
    // Members are inherited from JsonRepositoryBase<Restaurant>.
};

}  // namespace Persistence
}  // namespace RecipeApp

#endif  // JSON_RESTAURANT_REPOSITORY_H