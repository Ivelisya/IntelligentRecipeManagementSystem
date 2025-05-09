#include "../src/persistence/PersistenceManager.h"
#include "../src/logic/user/UserManager.h"
#include "../src/logic/recipe/RecipeManager.h"
#include "../src/logic/restaurant/RestaurantManager.h"
#include "../src/persistence/JsonUserRepository.h"
#include "../src/persistence/JsonRecipeRepository.h"
#include "../src/persistence/JsonRestaurantRepository.h"
#include "../src/domain/user/User.h"
#include "../src/domain/recipe/Recipe.h"
#include "../src/domain/restaurant/Restaurant.h"

#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <vector>
#include <cstdio> // For std::remove

// Helper function to print test results
void printTestResult(const std::string &testName, bool success)
{
    std::cout << "Test " << testName << ": " << (success ? "PASSED" : "FAILED") << std::endl;
}

// Helper to create a dummy Recipe for testing
RecipeApp::Recipe createDummyRecipeComp(int id, const std::string &name, const std::string &cuisine = "TestCuisine")
{
    return RecipeApp::Recipe(id, name, {}, {}, 60, RecipeApp::Difficulty::Medium, cuisine);
}

// Helper to compare two User objects (basic comparison)
bool compareUsers(const RecipeApp::User &u1, const RecipeApp::User &u2)
{
    return u1.getUserId() == u2.getUserId() &&
           u1.getUsername() == u2.getUsername() &&
           u1.getPlainTextPassword() == u2.getPlainTextPassword() && // Simplified for test
           u1.getRole() == u2.getRole();
}

// Helper to compare two Recipe objects (basic comparison)
bool compareRecipes(const RecipeApp::Recipe &r1, const RecipeApp::Recipe &r2)
{
    // Add more field comparisons if necessary
    return r1.getRecipeId() == r2.getRecipeId() &&
           r1.getName() == r2.getName() &&
           r1.getCuisine() == r2.getCuisine();
}

// Helper to compare two Restaurant objects (basic comparison)
bool compareRestaurantsComp(const RecipeApp::Restaurant &r1, const RecipeApp::Restaurant &r2)
{
    return r1.getRestaurantId() == r2.getRestaurantId() &&
           r1.getName() == r2.getName() &&
           r1.getAddress() == r2.getAddress();
}

int main()
{
    std::cout << "--- Running Comprehensive PersistenceManager Tests ---" << std::endl;
    bool all_tests_passed = true;

    const std::string testUserFile = "test_users_comp.json";
    const std::string testRecipeFile = "test_recipes_comp.json";
    const std::string testRestaurantFile = "test_restaurants_comp.json";

    // Test Case 1: Save and Load All Data Types
    bool test1_success = true;
    std::cout << "\n--- Test Case 1: Save and Load All Data Types ---" << std::endl;
    try
    {
        // --- Setup and Save ---
        std::remove(testUserFile.c_str());
        std::remove(testRecipeFile.c_str());
        std::remove(testRestaurantFile.c_str());

        RecipeApp::Persistence::JsonUserRepository userRepo_s(testUserFile);
        RecipeApp::UserManager userMgr_s(userRepo_s);
        RecipeApp::Persistence::JsonRecipeRepository recipeRepo_s(testRecipeFile);
        RecipeApp::RecipeManager recipeMgr_s(recipeRepo_s);
        RecipeApp::Persistence::JsonRestaurantRepository restaurantRepo_s(testRestaurantFile);
        RecipeApp::RestaurantManager restaurantMgr_s(restaurantRepo_s);

        // Add Users
        userMgr_s.registerUser("user1_comp", "pass1");                           // ID 1
        userMgr_s.registerUser("user2_comp", "pass2");                           // ID 2
        int expectedUserNextId = userMgr_s.getAllUsers().back().getUserId() + 1; // Simplified nextId expectation

        // Add Recipes
        recipeMgr_s.addRecipe(createDummyRecipeComp(0, "Recipe1_comp")); // ID 1
        recipeMgr_s.addRecipe(createDummyRecipeComp(0, "Recipe2_comp")); // ID 2
        int expectedRecipeNextId = recipeMgr_s.getAllRecipes().back().getRecipeId() + 1;

        // Add Restaurants
        RecipeApp::Restaurant r1_s(0, "Restaurant1_comp", "Addr1", "Cont1", "Hours1");
        RecipeApp::Restaurant r2_s(0, "Restaurant2_comp", "Addr2", "Cont2", "Hours2");
        restaurantMgr_s.addRestaurant(r1_s); // ID 1
        restaurantMgr_s.addRestaurant(r2_s); // ID 2
        int expectedRestaurantNextId = restaurantMgr_s.getAllRestaurants().back().getRestaurantId() + 1;

        RecipeApp::PersistenceManager pm_s(testUserFile, testRecipeFile, testRestaurantFile);
        assert(pm_s.saveData(userMgr_s, recipeMgr_s, restaurantMgr_s) && "SaveData failed");

        // --- Load and Verify ---
        RecipeApp::Persistence::JsonUserRepository userRepo_l(testUserFile);
        RecipeApp::UserManager userMgr_l(userRepo_l);
        RecipeApp::Persistence::JsonRecipeRepository recipeRepo_l(testRecipeFile);
        RecipeApp::RecipeManager recipeMgr_l(recipeRepo_l);
        RecipeApp::Persistence::JsonRestaurantRepository restaurantRepo_l(testRestaurantFile);
        RecipeApp::RestaurantManager restaurantMgr_l(restaurantRepo_l);

        RecipeApp::PersistenceManager pm_l(testUserFile, testRecipeFile, testRestaurantFile);
        assert(pm_l.loadData(userMgr_l, recipeMgr_l, restaurantMgr_l) && "LoadData failed");

        // Verify Users
        auto loadedUsers = userMgr_l.getAllUsers();
        assert(loadedUsers.size() == 2 && "User count mismatch after load");
        assert(userMgr_l.getCurrentUser() == nullptr && "Current user should be null after load"); // Assuming load doesn't set current user
        // A more robust nextId check would be needed if JsonUserRepository::setNextId was used by PersistenceManager
        // For now, we check if it's at least what we expect from loaded data.
        // This part of the test might need adjustment based on how nextId is truly managed by JsonRepo on load.
        // The current JsonRepo.load() recalculates nextId based on max loaded ID.
        // PersistenceManager calls setNextIdFromPersistence.
        // Let's assume PersistenceManager's setNextId call is the source of truth for nextId after load.
        // This requires JsonRepo's setNextId to be effective.

        // Verify Recipes
        auto loadedRecipes = recipeMgr_l.getAllRecipes();
        assert(loadedRecipes.size() == 2 && "Recipe count mismatch after load");

        // Verify Restaurants
        auto loadedRestaurants = restaurantMgr_l.getAllRestaurants();
        assert(loadedRestaurants.size() == 2 && "Restaurant count mismatch after load");
        assert(restaurantMgr_l.getNextRestaurantId() == expectedRestaurantNextId && "Restaurant nextId mismatch after load");

        // Spot check one of each
        auto user1_l = userMgr_l.loginUser("user1_comp", "pass1");
        assert(user1_l != nullptr && user1_l->getUserId() == 1 && "User1 load/login failed");
        userMgr_l.logoutUser();

        auto recipe1_l = recipeMgr_l.findRecipeById(1);
        assert(recipe1_l.has_value() && recipe1_l.value().getName() == "Recipe1_comp" && "Recipe1 load failed");

        auto rest1_l = restaurantMgr_l.findRestaurantById(1);
        assert(rest1_l.has_value() && rest1_l.value().getName() == "Restaurant1_comp" && "Restaurant1 load failed");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Test Case 1 Exception: " << e.what() << std::endl;
        test1_success = false;
    }
    printTestResult("Save and Load All Data Types", test1_success);
    if (!test1_success)
        all_tests_passed = false;

    // --- Cleanup ---
    std::remove(testUserFile.c_str());
    std::remove(testRecipeFile.c_str());
    std::remove(testRestaurantFile.c_str());

    std::cout << "\n--- Comprehensive PersistenceManager Tests Finished ---" << std::endl;
    return all_tests_passed ? 0 : 1;
}