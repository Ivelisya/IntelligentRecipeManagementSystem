#include "../src/persistence/PersistenceManager.h"
#include "../src/logic/user/UserManager.h"
#include "../src/logic/recipe/RecipeManager.h"
#include "../src/logic/restaurant/RestaurantManager.h"
#include "../src/domain/user/User.h"
#include "../src/domain/recipe/Recipe.h"
#include "../src/domain/restaurant/Restaurant.h"
#include "../src/persistence/JsonUserRepository.h"       // Added
#include "../src/persistence/JsonRecipeRepository.h"     // Added
#include "../src/persistence/JsonRestaurantRepository.h" // Added
#include <iostream>
#include <fstream> // For cleanup
#include <cassert>
#include <string>
#include <vector>

// Helper function from previous tests
void printTestResult(const std::string &testName, bool success)
{
    std::cout << "Test " << testName << ": " << (success ? "PASSED" : "FAILED") << std::endl;
}

// Helper to compare two Restaurant objects (basic comparison)
bool compareRestaurants(const RecipeApp::Restaurant &r1, const RecipeApp::Restaurant &r2)
{
    bool id_match = r1.getRestaurantId() == r2.getRestaurantId();
    bool name_match = r1.getName() == r2.getName();
    bool address_match = r1.getAddress() == r2.getAddress();
    bool contact_match = r1.getContact() == r2.getContact();
    bool hours_match = r1.getOpeningHours() == r2.getOpeningHours();
    bool recipes_match = r1.getFeaturedRecipeIds() == r2.getFeaturedRecipeIds();

    if (!id_match)
        std::cout << "DEBUG: ID mismatch: " << r1.getRestaurantId() << " vs " << r2.getRestaurantId() << std::endl
                  << std::flush;
    if (!name_match)
        std::cout << "DEBUG: Name mismatch: '" << r1.getName() << "' vs '" << r2.getName() << "'" << std::endl
                  << std::flush;
    if (!address_match)
        std::cout << "DEBUG: Address mismatch: '" << r1.getAddress() << "' vs '" << r2.getAddress() << "'" << std::endl
                  << std::flush;
    if (!contact_match)
        std::cout << "DEBUG: Contact mismatch: '" << r1.getContact() << "' vs '" << r2.getContact() << "'" << std::endl
                  << std::flush;
    if (!hours_match)
        std::cout << "DEBUG: Hours mismatch: '" << r1.getOpeningHours() << "' vs '" << r2.getOpeningHours() << "'" << std::endl
                  << std::flush;
    if (!recipes_match)
    {
        std::cout << "DEBUG: Recipe IDs mismatch." << std::endl
                  << std::flush;
        std::cout << "  R1 IDs (" << r1.getFeaturedRecipeIds().size() << "): ";
        for (int id : r1.getFeaturedRecipeIds())
            std::cout << id << " ";
        std::cout << std::endl
                  << std::flush;
        std::cout << "  R2 IDs (" << r2.getFeaturedRecipeIds().size() << "): ";
        for (int id : r2.getFeaturedRecipeIds())
            std::cout << id << " ";
        std::cout << std::endl
                  << std::flush;
    }

    return id_match && name_match && address_match && contact_match && hours_match && recipes_match;
}

int main()
{
    std::cout << "--- Running PersistenceManager Restaurant Integration Tests ---" << std::endl;

    const std::string testUserFile = "test_users_persist_rest.json";
    const std::string testRecipeFile = "test_recipes_persist_rest.json";
    const std::string testRestaurantFile = "test_restaurants_persist.json"; // Specific file for this test

    // Clean up previous test files if they exist
    std::remove(testUserFile.c_str());
    std::remove(testRecipeFile.c_str());
    std::remove(testRestaurantFile.c_str());

    bool test_save_load_success = true;
    try
    {
        // --- Setup and Save ---
        RecipeApp::Persistence::JsonUserRepository userRepo_save(testUserFile);
        RecipeApp::UserManager userMgr_save(userRepo_save); // Keep empty for this test focus

        RecipeApp::Persistence::JsonRecipeRepository recipeRepo_save(testRecipeFile);
        RecipeApp::RecipeManager recipeMgr_save(recipeRepo_save); // Keep empty for this test focus

        RecipeApp::Persistence::JsonRestaurantRepository restaurantRepo_save(testRestaurantFile);
        RecipeApp::RestaurantManager restaurantMgr_save(restaurantRepo_save);

        RecipeApp::Restaurant r1_in(0, "Persist Cafe", "Addr Persist 1", "Cont P1", "Hours P1");
        r1_in.addFeaturedRecipe(10);
        r1_in.addFeaturedRecipe(20);
        restaurantMgr_save.addRestaurant(r1_in); // Gets ID 1

        RecipeApp::Restaurant r2_in(0, "Data Diner", "Addr Persist 2", "Cont P2", "Hours P2");
        r2_in.addFeaturedRecipe(30);
        restaurantMgr_save.addRestaurant(r2_in); // Gets ID 2

        int expectedNextId = restaurantMgr_save.getNextRestaurantId(); // Should be 3

        RecipeApp::PersistenceManager pm_save(testUserFile, testRecipeFile, testRestaurantFile);
        bool saveResult = pm_save.saveData(userMgr_save, recipeMgr_save, restaurantMgr_save);
        assert(saveResult);

        // --- Load and Verify ---
        RecipeApp::Persistence::JsonUserRepository userRepo_load(testUserFile);
        RecipeApp::UserManager userMgr_load(userRepo_load);

        RecipeApp::Persistence::JsonRecipeRepository recipeRepo_load(testRecipeFile);
        RecipeApp::RecipeManager recipeMgr_load(recipeRepo_load);

        RecipeApp::Persistence::JsonRestaurantRepository restaurantRepo_load(testRestaurantFile);
        RecipeApp::RestaurantManager restaurantMgr_load(restaurantRepo_load);

        RecipeApp::PersistenceManager pm_load(testUserFile, testRecipeFile, testRestaurantFile);
        bool loadResult = pm_load.loadData(userMgr_load, recipeMgr_load, restaurantMgr_load);
        assert(loadResult);

        // Verify RestaurantManager state
        assert(restaurantMgr_load.getNextRestaurantId() == expectedNextId);
        const auto &loadedRestaurants = restaurantMgr_load.getAllRestaurants();
        assert(loadedRestaurants.size() == 2);

        // Verify individual restaurants (order might not be guaranteed by list, so find by ID)
        std::optional<RecipeApp::Restaurant> loaded_r1_opt = restaurantMgr_load.findRestaurantById(1);
        std::optional<RecipeApp::Restaurant> loaded_r2_opt = restaurantMgr_load.findRestaurantById(2);

        assert(loaded_r1_opt.has_value());
        assert(loaded_r2_opt.has_value());

        // Create expected versions of r1 and r2 with assigned IDs
        RecipeApp::Restaurant expected_r1(1, "Persist Cafe", "Addr Persist 1", "Cont P1", "Hours P1");
        expected_r1.addFeaturedRecipe(10);
        expected_r1.addFeaturedRecipe(20);

        RecipeApp::Restaurant expected_r2(2, "Data Diner", "Addr Persist 2", "Cont P2", "Hours P2");
        expected_r2.addFeaturedRecipe(30);

        assert(compareRestaurants(loaded_r1_opt.value(), expected_r1));
        assert(compareRestaurants(loaded_r2_opt.value(), expected_r2));
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception during test: " << e.what() << std::endl;
        test_save_load_success = false;
    }
    catch (...)
    {
        test_save_load_success = false;
    }

    printTestResult("Save/Load Restaurant Data", test_save_load_success);

    // --- Cleanup ---
    std::remove(testUserFile.c_str());
    std::remove(testRecipeFile.c_str());
    std::remove(testRestaurantFile.c_str());

    std::cout << "--- PersistenceManager Restaurant Integration Tests Finished ---" << std::endl;

    return test_save_load_success ? 0 : 1;
}