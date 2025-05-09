#include "persistence/JsonRestaurantRepository.h"
#include "domain/restaurant/Restaurant.h" // Ensure Restaurant class definition is included
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>    // For string stream helpers
#include <algorithm>  // For std::find_if, std::remove_if
#include <filesystem> // For path operations and directory creation

namespace RecipeApp
{
    namespace Persistence
    {
        // Use the fully qualified name or a using declaration
        using ::RecipeApp::Restaurant;

        // Constructor
        JsonRestaurantRepository::JsonRestaurantRepository(const std::filesystem::path &baseDirectory, const std::string &fileName)
            : m_nextId(1)
        {
            // Ensure the base directory exists
            if (!std::filesystem::exists(baseDirectory))
            {
                try
                {
                    if (std::filesystem::create_directories(baseDirectory))
                    {
                        // Optional: Log directory creation
                        // std::cout << "Info: Created base directory: " << baseDirectory.string() << std::endl;
                    }
                }
                catch (const std::filesystem::filesystem_error &e)
                {
                    std::cerr << "Critical Error: Could not create base directory '" << baseDirectory.string() << "': " << e.what() << std::endl;
                    // Fallback to current directory with just filename, though not ideal for explicit paths.
                    m_filePath = fileName;
                    std::cerr << "Warning: Using fallback file path in current directory for restaurants: " << m_filePath << std::endl;
                    return;
                }
            }

            m_filePath = (baseDirectory / fileName).string();
            // Optional: Log the final path being used
            // std::cout << "Info: Using restaurant data file: " << m_filePath << std::endl;
        }

        // --- Serialization/Deserialization Helpers ---

        // Helper to serialize vector<int> to a delimited string (e.g., "1;2;3")
        std::string JsonRestaurantRepository::serializeFeaturedRecipeIds(const std::vector<int> &ids) const
        {
            std::ostringstream oss;
            for (size_t i = 0; i < ids.size(); ++i)
            {
                oss << ids[i];
                if (i < ids.size() - 1)
                {
                    oss << ";"; // Use semicolon as delimiter
                }
            }
            return oss.str();
        }

        // Helper to deserialize a delimited string back to vector<int>
        std::vector<int> JsonRestaurantRepository::deserializeFeaturedRecipeIds(const std::string &s) const
        {
            std::vector<int> ids;
            if (s.empty())
                return ids;
            std::string segment;
            std::istringstream stream(s);
            while (std::getline(stream, segment, ';'))
            {
                try
                {
                    ids.push_back(std::stoi(segment));
                }
                catch (const std::invalid_argument &ia)
                {
                    (void)ia; // Mark as intentionally unused
                    std::cerr << "Warning: Invalid integer '" << segment << "' found while deserializing featured recipe ID. Skipped." << std::endl;
                }
                catch (const std::out_of_range &oor)
                {
                    (void)oor; // Mark as intentionally unused
                    std::cerr << "Warning: Out-of-range integer '" << segment << "' found while deserializing featured recipe ID. Skipped." << std::endl;
                }
            }
            return ids;
        }

        json JsonRestaurantRepository::restaurantToJson(const Restaurant &restaurant) const
        {
            json j;
            j["id"] = restaurant.getRestaurantId();
            j["name"] = restaurant.getName();
            j["address"] = restaurant.getAddress();
            j["contact"] = restaurant.getContact();
            j["openingHours"] = restaurant.getOpeningHours();
            j["featuredRecipeIds"] = serializeFeaturedRecipeIds(restaurant.getFeaturedRecipeIds());
            return j;
        }

        Restaurant JsonRestaurantRepository::jsonToRestaurant(const json &j) const
        {
            int id = j.value("id", 0);
            std::string name = j.value("name", "");
            std::string address = j.value("address", "");
            std::string contact = j.value("contact", "");
            std::string openingHours = j.value("openingHours", "");
            std::string featuredIdsStr = j.value("featuredRecipeIds", "");

            Restaurant restaurant(id, name, address, contact, openingHours);

            // Deserialize and add featured recipe IDs
            std::vector<int> featuredIds = deserializeFeaturedRecipeIds(featuredIdsStr);
            for (int recipeId : featuredIds)
            {
                restaurant.addFeaturedRecipe(recipeId); // Use the Restaurant's method to add IDs
            }

            return restaurant;
        }

        // --- Load/Save Operations ---

        bool JsonRestaurantRepository::load()
        {
            std::ifstream file(m_filePath);
            if (!file.is_open())
            {
                std::cerr << "Warning: Could not open restaurant data file for reading: " << m_filePath << ". Starting with an empty restaurant list." << std::endl;
                m_restaurants.clear();
                m_nextId = 1;
                return true; // Okay to start empty
            }

            try
            {
                json data = json::parse(file);
                file.close();

                m_restaurants.clear();
                int maxId = 0;

                if (data.contains("restaurants") && data["restaurants"].is_array())
                {
                    for (const auto &restaurantJson : data["restaurants"])
                    {
                        Restaurant restaurant = jsonToRestaurant(restaurantJson);
                        if (restaurant.getRestaurantId() > 0)
                        {
                            m_restaurants.push_back(restaurant);
                            if (restaurant.getRestaurantId() > maxId)
                            {
                                maxId = restaurant.getRestaurantId();
                            }
                        }
                        else
                        {
                            std::cerr << "Warning: Invalid restaurant ID (<=0) found while loading restaurants. Skipped." << std::endl;
                        }
                    }
                }
                m_nextId = maxId + 1;
                return true;
            }
            catch (const json::parse_error &e)
            {
                std::cerr << "Error: Failed to parse restaurant JSON data: " << e.what() << " at byte " << e.byte << std::endl;
                m_restaurants.clear();
                m_nextId = 1;
                return false;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error loading restaurant data: " << e.what() << std::endl;
                m_restaurants.clear();
                m_nextId = 1;
                return false;
            }
        }

        bool JsonRestaurantRepository::saveAll()
        {
            json dataDoc; // Changed variable name
            json restaurantsJsonArray = json::array();

            for (const auto &restaurant : m_restaurants)
            {
                restaurantsJsonArray.push_back(restaurantToJson(restaurant));
            }
            dataDoc["restaurants"] = restaurantsJsonArray;

            std::filesystem::path filePathObj(m_filePath);
            std::filesystem::path tempFilePathObj = filePathObj;
            tempFilePathObj += ".tmp";

            if (!filePathObj.parent_path().empty() && !std::filesystem::exists(filePathObj.parent_path()))
            {
                try
                {
                    std::filesystem::create_directories(filePathObj.parent_path());
                }
                catch (const std::filesystem::filesystem_error &e)
                {
                    std::cerr << "Error: Could not create directory for saving file "
                              << filePathObj.parent_path().string() << ": " << e.what() << std::endl;
                    return false;
                }
            }

            std::ofstream tempFile(tempFilePathObj.string(), std::ios::out | std::ios::trunc);
            if (!tempFile.is_open())
            {
                std::cerr << "Error: Could not open temporary restaurant data file for writing: " << tempFilePathObj.string() << std::endl;
                return false;
            }

            try
            {
                tempFile << dataDoc.dump(2);
                tempFile.close();

                if (tempFile.fail())
                {
                    std::cerr << "Error: Failed to write all data to temporary file: " << tempFilePathObj.string() << std::endl;
                    std::filesystem::remove(tempFilePathObj);
                    return false;
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error serializing restaurant data to temporary file '" << tempFilePathObj.string() << "': " << e.what() << std::endl;
                if (tempFile.is_open())
                    tempFile.close();
                std::filesystem::remove(tempFilePathObj);
                return false;
            }

            try
            {
                std::filesystem::rename(tempFilePathObj, filePathObj);
                return true;
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                std::cerr << "Error: Failed to replace original file '" << filePathObj.string()
                          << "' with temporary file '" << tempFilePathObj.string() << "': " << e.what() << std::endl;
                if (std::filesystem::exists(tempFilePathObj))
                {
                    std::error_code ec;
                    std::filesystem::remove(tempFilePathObj, ec);
                    if (ec)
                    {
                        std::cerr << "Warning: Could not remove temporary file '" << tempFilePathObj.string() << "' after rename failed: " << ec.message() << std::endl;
                    }
                }
                return false;
            }
        }

        // --- RestaurantRepository Interface Implementation ---

        std::optional<Restaurant> JsonRestaurantRepository::findById(int restaurantId) const
        {
            for (const auto &restaurant : m_restaurants)
            {
                if (restaurant.getRestaurantId() == restaurantId)
                {
                    return restaurant; // Return a copy
                }
            }
            return std::nullopt;
        }

        std::vector<Restaurant> JsonRestaurantRepository::findByName(const std::string &name, bool partialMatch) const
        {
            std::vector<Restaurant> results;
            std::string lowerName = name;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower); // Enabled case-insensitivity

            for (const auto &restaurant : m_restaurants)
            {
                std::string currentRestaurantName = restaurant.getName();
                std::transform(currentRestaurantName.begin(), currentRestaurantName.end(), currentRestaurantName.begin(), ::tolower); // Enabled case-insensitivity

                if (partialMatch)
                {
                    if (currentRestaurantName.find(lowerName) != std::string::npos)
                    {
                        results.push_back(restaurant); // Add a copy
                    }
                }
                else
                {
                    if (currentRestaurantName == lowerName)
                    {
                        results.push_back(restaurant); // Add a copy
                    }
                }
            }
            return results;
        }

        std::vector<Restaurant> JsonRestaurantRepository::findAll() const
        {
            std::vector<Restaurant> allRestaurants;
            allRestaurants.reserve(m_restaurants.size());
            for (const auto &restaurant : m_restaurants)
            {
                allRestaurants.push_back(restaurant);
            }
            return allRestaurants;
        }

        int JsonRestaurantRepository::save(const Restaurant &restaurantToSave)
        {
            int idToUse = restaurantToSave.getRestaurantId();
            bool existingFoundAndUpdated = false;

            if (idToUse > 0)
            { // If a specific ID is provided, try to find and update
                for (auto it = m_restaurants.begin(); it != m_restaurants.end(); ++it)
                {
                    if ((*it).getRestaurantId() == idToUse)
                    {
                        *it = restaurantToSave; // Update existing restaurant
                        existingFoundAndUpdated = true;
                        break;
                    }
                }
            }

            if (!existingFoundAndUpdated)
            {                                                  // If not found (either ID was <=0, or ID > 0 but not in list - e.g. loading from persistence)
                Restaurant restaurantToAdd = restaurantToSave; // Start with a copy

                if (idToUse <= 0)
                { // It's a truly new restaurant, assign a new ID from m_nextId
                    idToUse = m_nextId++;
                    // Create a new Restaurant object with the new ID and copy other fields
                    restaurantToAdd = Restaurant(idToUse,
                                                 restaurantToSave.getName(),
                                                 restaurantToSave.getAddress(),
                                                 restaurantToSave.getContact(),
                                                 restaurantToSave.getOpeningHours());
                    for (int recipeId : restaurantToSave.getFeaturedRecipeIds())
                    {
                        restaurantToAdd.addFeaturedRecipe(recipeId);
                    }
                }
                // If idToUse > 0 but not found, it means we are loading from persistence.
                // We use the idToUse (which is restaurantToSave.getRestaurantId())
                // and restaurantToAdd is already a correct copy.
                m_restaurants.push_back(restaurantToAdd);
            }

            // Ensure m_nextId is always greater than the ID just processed or any existing max ID
            if (idToUse >= m_nextId)
            {
                m_nextId = idToUse + 1;
            }
            // Additionally, ensure m_nextId is greater than any ID in the list (e.g. after loading)
            for (const auto &r : m_restaurants)
            {
                if (r.getRestaurantId() >= m_nextId)
                {
                    m_nextId = r.getRestaurantId() + 1;
                }
            }

            if (saveAll())
            {
                return idToUse; // Return the ID used for saving/updating
            }
            else
            {
                std::cerr << "Error: saveAll() failed after JsonRestaurantRepository::save()." << std::endl;
                return -1; // Indicate failure
            }
        }

        bool JsonRestaurantRepository::remove(int restaurantId)
        {
            auto initial_size = m_restaurants.size();
            m_restaurants.erase(
                std::remove_if(m_restaurants.begin(), m_restaurants.end(),
                               [restaurantId](const Restaurant &restaurant)
                               {
                                   return restaurant.getRestaurantId() == restaurantId;
                               }),
                m_restaurants.end());

            if (m_restaurants.size() < initial_size) // If an element was removed
            {
                return saveAll();
            }
            return false; // Restaurant not found or not removed
        }

        // getNextId() const implementation (optional)
        // int JsonRestaurantRepository::getNextId() const { return m_nextId; }

    } // namespace Persistence
} // namespace RecipeApp
// Add new method implementation at the end of the file, before the closing namespace brace.

int RecipeApp::Persistence::JsonRestaurantRepository::getNextId() const
{
    return m_nextId;
}
// Add new method implementation at the end of the file, before the closing namespace brace.

void RecipeApp::Persistence::JsonRestaurantRepository::setNextId(int nextId)
{
    m_nextId = nextId;
}