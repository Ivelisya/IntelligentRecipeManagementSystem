#include "PersistenceManager.h"
#include "domain/restaurant/Restaurant.h" // Required for Restaurant object usage, Updated path
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <algorithm> // For std::remove
#include <optional>  // For std::optional in Recipe

namespace RecipeApp
{
    // Helper to split a string by a delimiter
    std::vector<std::string> split(const std::string &s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    // Helper to join a vector of strings with a delimiter
    std::string join(const std::vector<std::string> &elements, const std::string &delimiter)
    {
        std::ostringstream os;
        for (size_t i = 0; i < elements.size(); ++i)
        {
            os << elements[i];
            if (i < elements.size() - 1)
            {
                os << delimiter;
            }
        }
        return os.str();
    }

    std::string serializeIngredients(const std::vector<std::pair<std::string, std::string>> &ingredients)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < ingredients.size(); ++i)
        {
            std::string item = ingredients[i].first;
            std::string quantity = ingredients[i].second;
            // Basic escaping for problematic characters if they were allowed in names/quantities
            // For simplicity, assuming they don't contain ';' or ':'
            oss << item << ":" << quantity;
            if (i < ingredients.size() - 1)
            {
                oss << ";";
            }
        }
        return oss.str();
    }

    std::vector<std::pair<std::string, std::string>> deserializeIngredients(const std::string &s)
    {
        std::vector<std::pair<std::string, std::string>> ingredients;
        if (s.empty())
            return ingredients;
        std::vector<std::string> pairs = split(s, ';');
        for (const auto &pair_str : pairs)
        {
            std::vector<std::string> item_quantity = split(pair_str, ':');
            if (item_quantity.size() == 2)
            {
                ingredients.push_back({item_quantity[0], item_quantity[1]});
            }
        }
        return ingredients;
    }

    std::string serializeSteps(const std::vector<std::string> &steps)
    {
        return join(steps, "@@STEP@@");
    }

    std::vector<std::string> deserializeSteps(const std::string &s)
    {
        std::vector<std::string> result;
        if (s.empty())
            return result;
        size_t start = 0;
        size_t end = s.find("@@STEP@@");
        while (end != std::string::npos)
        {
            result.push_back(s.substr(start, end - start));
            start = end + 8; // Length of "@@STEP@@"
            end = s.find("@@STEP@@", start);
        }
        result.push_back(s.substr(start));
        return result;
    }

    std::string difficultyToString(Difficulty d)
    {
        switch (d)
        {
        case Difficulty::Easy:
            return "Easy";
        case Difficulty::Medium:
            return "Medium";
        case Difficulty::Hard:
            return "Hard";
        default:
            return "Unknown";
        }
    }

    Difficulty stringToDifficulty(const std::string &s)
    {
        if (s == "Easy")
            return Difficulty::Easy;
        if (s == "Medium")
            return Difficulty::Medium;
        if (s == "Hard")
            return Difficulty::Hard;
        return Difficulty::Easy; // Default
    }

    std::string serializeIntVector(const std::vector<int> &vec)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < vec.size(); ++i)
        {
            oss << vec[i] << (i == vec.size() - 1 ? "" : ",");
        }
        return oss.str();
    }

    std::vector<int> deserializeIntVector(const std::string &s)
    {
        std::vector<int> vec;
        if (s.empty())
            return vec;
        std::vector<std::string> tokens = split(s, ',');
        for (const auto &token : tokens)
        {
            if (!token.empty())
            {
                try
                {
                    vec.push_back(std::stoi(token));
                }
                catch (const std::exception &e)
                { // Catch generic std::exception
                    std::cerr << "警告: 反序列化整数 '" << token << "' 失败: " << e.what() << std::endl;
                }
            }
        }
        return vec;
    }

    PersistenceManager::PersistenceManager(const std::string &userPath, const std::string &recipePath, const std::string &restaurantPath)
        : userFilePath(userPath), recipeFilePath(recipePath), restaurantFilePath(restaurantPath)
    {
    }

    bool PersistenceManager::saveData(const UserManager &userManager, const RecipeManager &recipeManager, const RestaurantManager &restaurantManager) const
    {
        try
        {
            // 保存用户数据
            std::ofstream userFile(userFilePath);
            if (!userFile.is_open())
            {
                throw std::runtime_error("无法打开用户数据文件进行写入: " + userFilePath);
            }
            userFile << "{\n";
            userFile << "  \"users\": [\n";
            const auto &users = userManager.getAllUsers();
            bool firstUser = true;
            for (const auto &user : users)
            {
                if (!firstUser)
                    userFile << ",\n";
                userFile << "    {\n";
                userFile << "      \"id\": " << user.getUserId() << ",\n";
                userFile << "      \"username\": \"" << user.getUsername() << "\",\n";
                userFile << "      \"password\": \"" << user.getPlainTextPassword() << "\"\n"; // Corrected to getPlainTextPassword
                userFile << "    }";
                firstUser = false;
            }
            if (!users.empty())
                userFile << "\n";
            userFile << "  ]\n";
            userFile << "}\n";
            userFile.close();

            // 保存食谱数据
            std::ofstream recipeFile(recipeFilePath);
            if (!recipeFile.is_open())
            {
                throw std::runtime_error("无法打开食谱数据文件进行写入: " + recipeFilePath);
            }
            recipeFile << "{\n";
            recipeFile << "  \"recipes\": [\n";
            const auto &recipes = recipeManager.getAllRecipes();
            bool firstRecipe = true;
            for (const auto &recipe : recipes)
            {
                if (!firstRecipe)
                    recipeFile << ",\n";
                recipeFile << "    {\n";
                recipeFile << "      \"id\": " << recipe.getRecipeId() << ",\n";
                recipeFile << "      \"name\": \"" << recipe.getName() << "\",\n";
                recipeFile << "      \"ingredients\": \"" << serializeIngredients(recipe.getIngredients()) << "\",\n";
                recipeFile << "      \"steps\": \"" << serializeSteps(recipe.getSteps()) << "\",\n";
                recipeFile << "      \"cookingTime\": " << recipe.getCookingTime() << ",\n";
                recipeFile << "      \"difficulty\": \"" << difficultyToString(recipe.getDifficulty()) << "\",\n";
                recipeFile << "      \"cuisine\": \"" << recipe.getCuisine() << "\"";
                if (recipe.getNutritionalInfo().has_value())
                {
                    recipeFile << ",\n      \"nutritionalInfo\": \"" << recipe.getNutritionalInfo().value() << "\"";
                }
                if (recipe.getImageUrl().has_value())
                {
                    recipeFile << ",\n      \"imageUrl\": \"" << recipe.getImageUrl().value() << "\"";
                }
                recipeFile << "\n    }";
                firstRecipe = false;
            }
            if (!recipes.empty())
                recipeFile << "\n";
            recipeFile << "  ]\n";
            recipeFile << "}\n";
            recipeFile.close();

            // 保存餐厅数据
            std::ofstream restaurantFile(restaurantFilePath);
            if (!restaurantFile.is_open())
            {
                throw std::runtime_error("无法打开餐厅数据文件进行写入: " + restaurantFilePath);
            }
            restaurantFile << "{\n";
            restaurantFile << "  \"nextRestaurantId\": " << restaurantManager.getNextRestaurantId() << ",\n";
            restaurantFile << "  \"restaurants\": [\n";
            const auto &restaurants = restaurantManager.getAllRestaurants();
            bool firstRestaurant = true;
            for (const auto &restaurant : restaurants)
            {
                if (!firstRestaurant)
                    restaurantFile << ",\n";
                restaurantFile << "    {\n";
                restaurantFile << "      \"id\": " << restaurant.getRestaurantId() << ",\n";
                restaurantFile << "      \"name\": \"" << restaurant.getName() << "\",\n";
                restaurantFile << "      \"address\": \"" << restaurant.getAddress() << "\",\n";
                restaurantFile << "      \"contact\": \"" << restaurant.getContact() << "\",\n";
                restaurantFile << "      \"openingHours\": \"" << restaurant.getOpeningHours() << "\",\n";
                restaurantFile << "      \"featuredRecipeIds\": \"" << serializeIntVector(restaurant.getFeaturedRecipeIds()) << "\"\n";
                restaurantFile << "    }";
                firstRestaurant = false;
            }
            if (!restaurants.empty())
                restaurantFile << "\n";
            restaurantFile << "  ]\n";
            restaurantFile << "}\n";
            restaurantFile.close();

            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "保存数据时发生错误: " << e.what() << std::endl;
            return false;
        }
    }

    // Helper to find a JSON value for a given key in a string segment
    std::string findJsonValue(const std::string &jsonData, const std::string &key, size_t &currentPos, size_t arrayEnd)
    {
        size_t keyPos = jsonData.find("\"" + key + "\": \"", currentPos); // Restored space after colon
        if (keyPos == std::string::npos || keyPos > arrayEnd)
            return ""; // Key not found or out of scope

        size_t valueStart = keyPos + key.length() + 5; // Past "key": " (e.g. past "name": ")
        size_t valueEnd = jsonData.find("\"", valueStart);
        if (valueEnd == std::string::npos || valueEnd > arrayEnd)
            return ""; // Value end quote not found or out of scope

        currentPos = valueEnd + 1; // Move past the value and its closing quote
        return jsonData.substr(valueStart, valueEnd - valueStart);
    }
    // Overload for integer values
    int findJsonIntValue(const std::string &jsonData, const std::string &key, size_t &currentPos, size_t arrayEnd, int defaultValue = 0)
    {
        size_t keyPos = jsonData.find("\"" + key + "\": ", currentPos); // Note: no quote after colon for numbers
        if (keyPos == std::string::npos || keyPos > arrayEnd)
            return defaultValue;

        size_t valueStart = keyPos + key.length() + 3;              // Past "...":
        size_t valueEnd = jsonData.find_first_of(",}", valueStart); // Value ends at comma or closing brace
        if (valueEnd == std::string::npos || valueEnd > arrayEnd)
            return defaultValue;

        currentPos = valueEnd; // Move to the end of the value
        try
        {
            return std::stoi(jsonData.substr(valueStart, valueEnd - valueStart));
        }
        catch (const std::exception &e)
        {
            std::cerr << "警告: 解析JSON整数值 '" << key << "' 失败: " << e.what() << std::endl;
            return defaultValue;
        }
    }

    bool PersistenceManager::loadData(UserManager &userManager, RecipeManager &recipeManager, RestaurantManager &restaurantManager) const
    {
        try
        {
            // 加载用户数据
            std::ifstream userFile(userFilePath);
            if (userFile.is_open())
            {
                std::stringstream userBuffer;
                userBuffer << userFile.rdbuf();
                std::string userData = userBuffer.str();
                userFile.close();
                int maxUserId = 0;
                size_t usersArrayStart = userData.find("\"users\": [");
                if (usersArrayStart != std::string::npos)
                {
                    size_t currentPos = usersArrayStart + 10;
                    size_t usersArrayEnd = userData.find("]", currentPos);
                    if (usersArrayEnd == std::string::npos)
                        usersArrayEnd = userData.length();

                    while (currentPos < usersArrayEnd && userData.find("{", currentPos) != std::string::npos)
                    {
                        size_t objectStart = userData.find("{", currentPos);
                        if (objectStart == std::string::npos || objectStart >= usersArrayEnd)
                            break;
                        currentPos = objectStart + 1;

                        int id = findJsonIntValue(userData, "id", currentPos, usersArrayEnd, 0);
                        std::string username = findJsonValue(userData, "username", currentPos, usersArrayEnd);
                        std::string password = findJsonValue(userData, "password", currentPos, usersArrayEnd);

                        if (id > 0)
                        { // Basic validation
                            if (id > maxUserId)
                                maxUserId = id;
                            User user(id, username, password);
                            userManager.addUserFromPersistence(user);
                        }
                        size_t objectEnd = userData.find("}", currentPos);
                        if (objectEnd == std::string::npos || objectEnd >= usersArrayEnd)
                            break;
                        currentPos = objectEnd + 1;
                    }
                }
                userManager.setNextUserIdFromPersistence(maxUserId + 1);
            }
            else
            {
                std::cerr << "警告: 无法打开用户数据文件进行读取: " << userFilePath << std::endl;
            }

            // 加载食谱数据
            std::ifstream recipeFile(recipeFilePath);
            if (recipeFile.is_open())
            {
                std::stringstream recipeBuffer;
                recipeBuffer << recipeFile.rdbuf();
                std::string recipeData = recipeBuffer.str();
                recipeFile.close();
                int maxRecipeId = 0;

                size_t recipesArrayStart = recipeData.find("\"recipes\": [");
                if (recipesArrayStart != std::string::npos)
                {
                    size_t currentPos = recipesArrayStart + 12;
                    size_t recipesArrayEnd = recipeData.find("]", currentPos);
                    if (recipesArrayEnd == std::string::npos)
                        recipesArrayEnd = recipeData.length();

                    while (currentPos < recipesArrayEnd && recipeData.find("{", currentPos) != std::string::npos)
                    {
                        size_t objectStart = recipeData.find("{", currentPos);
                        if (objectStart == std::string::npos || objectStart >= recipesArrayEnd)
                            break;
                        currentPos = objectStart + 1;

                        int id = findJsonIntValue(recipeData, "id", currentPos, recipesArrayEnd, 0);
                        std::string name = findJsonValue(recipeData, "name", currentPos, recipesArrayEnd);
                        std::string ingredients_str = findJsonValue(recipeData, "ingredients", currentPos, recipesArrayEnd);
                        std::string steps_str = findJsonValue(recipeData, "steps", currentPos, recipesArrayEnd);
                        int cookingTime = findJsonIntValue(recipeData, "cookingTime", currentPos, recipesArrayEnd, 0);
                        std::string difficulty_str = findJsonValue(recipeData, "difficulty", currentPos, recipesArrayEnd);
                        std::string cuisine = findJsonValue(recipeData, "cuisine", currentPos, recipesArrayEnd);
                        std::string nutritionalInfo_str = findJsonValue(recipeData, "nutritionalInfo", currentPos, recipesArrayEnd);
                        std::string imageUrl_str = findJsonValue(recipeData, "imageUrl", currentPos, recipesArrayEnd);

                        if (id > 0)
                        { // Basic validation
                            if (id > maxRecipeId)
                                maxRecipeId = id;
                            std::vector<std::pair<std::string, std::string>> ingredients = deserializeIngredients(ingredients_str);
                            std::vector<std::string> steps = deserializeSteps(steps_str);
                            Difficulty difficulty = stringToDifficulty(difficulty_str);

                            Recipe recipe(id, name, ingredients, steps, cookingTime, difficulty, cuisine);
                            if (!nutritionalInfo_str.empty())
                                recipe.setNutritionalInfo(nutritionalInfo_str);
                            if (!imageUrl_str.empty())
                                recipe.setImageUrl(imageUrl_str);
                            recipeManager.addRecipeFromPersistence(recipe);
                        }
                        size_t objectEnd = recipeData.find("}", currentPos);
                        if (objectEnd == std::string::npos || objectEnd >= recipesArrayEnd)
                            break;
                        currentPos = objectEnd + 1;
                    }
                }
                recipeManager.setNextRecipeIdFromPersistence(maxRecipeId + 1);
            }
            else
            {
                std::cerr << "警告: 无法打开食谱数据文件进行读取: " << recipeFilePath << std::endl;
            }

            // 加载餐厅数据
            std::ifstream restaurantFile(restaurantFilePath);
            if (restaurantFile.is_open())
            {
                std::stringstream restaurantBuffer;
                restaurantBuffer << restaurantFile.rdbuf();
                std::string restaurantData = restaurantBuffer.str();
                restaurantFile.close();

                // DEBUG PRINT REMOVED
                int nextRestIdFromFile = 1;
                size_t nextIdJsonPos = 0; // Dummy start position for findJsonIntValue
                nextRestIdFromFile = findJsonIntValue(restaurantData, "nextRestaurantId", nextIdJsonPos, restaurantData.length(), 1);
                restaurantManager.setNextRestaurantIdFromPersistence(nextRestIdFromFile);

                size_t restaurantsArrayStart = restaurantData.find("\"restaurants\": [");
                if (restaurantsArrayStart != std::string::npos)
                {
                    size_t currentPos = restaurantsArrayStart + 16;
                    size_t restaurantsArrayEnd = restaurantData.find("]", currentPos);
                    if (restaurantsArrayEnd == std::string::npos)
                        restaurantsArrayEnd = restaurantData.length();

                    while (currentPos < restaurantsArrayEnd && restaurantData.find("{", currentPos) != std::string::npos)
                    {
                        size_t objectStart = restaurantData.find("{", currentPos);
                        if (objectStart == std::string::npos || objectStart >= restaurantsArrayEnd)
                            break;
                        currentPos = objectStart + 1;

                        int id = findJsonIntValue(restaurantData, "id", currentPos, restaurantsArrayEnd, 0);
                        std::string name = findJsonValue(restaurantData, "name", currentPos, restaurantsArrayEnd);
                        std::string address = findJsonValue(restaurantData, "address", currentPos, restaurantsArrayEnd);
                        std::string contact = findJsonValue(restaurantData, "contact", currentPos, restaurantsArrayEnd);
                        std::string openingHours = findJsonValue(restaurantData, "openingHours", currentPos, restaurantsArrayEnd);
                        std::string fr_ids_str = findJsonValue(restaurantData, "featuredRecipeIds", currentPos, restaurantsArrayEnd);

                        if (id > 0)
                        { // Basic validation
                            Restaurant restaurant(id, name, address, contact, openingHours);
                            std::vector<int> featuredRecipeIds = deserializeIntVector(fr_ids_str);
                            for (int recipeId : featuredRecipeIds)
                            {
                                restaurant.addFeaturedRecipe(recipeId);
                            }
                            restaurantManager.addRestaurantFromPersistence(restaurant);
                        }
                        size_t objectEnd = restaurantData.find("}", currentPos);
                        if (objectEnd == std::string::npos || objectEnd >= restaurantsArrayEnd)
                            break;
                        currentPos = objectEnd + 1;
                    }
                }
            }
            else
            {
                std::cerr << "警告: 无法打开餐厅数据文件进行读取: " << restaurantFilePath << std::endl;
            }
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "加载数据时发生严重错误: " << e.what() << std::endl;
            return false; // Indicate failure on critical error during load
        }
    }

} // namespace RecipeApp