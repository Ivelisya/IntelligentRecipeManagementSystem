#include "../src/logic/restaurant/RestaurantManager.h"
#include "../src/domain/restaurant/Restaurant.h"
#include "../src/logic/recipe/RecipeManager.h" // Needed for getFeaturedRecipes test
#include "../src/domain/recipe/Recipe.h"       // Needed for Recipe objects
#include "../src/core/CustomLinkedList.h"
#include "../src/persistence/JsonRestaurantRepository.h" // Added for testing
#include "../src/persistence/JsonRecipeRepository.h"     // Added for testing
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

// Helper function from TestRestaurant.cpp (or move to a common test utility header)
void printTestResult(const std::string &testName, bool success)
{
    std::cout << "Test " << testName << ": " << (success ? "PASSED" : "FAILED") << std::endl;
}

// Helper to create a dummy Recipe for testing getFeaturedRecipes
RecipeApp::Recipe createDummyRecipe(int id, const std::string &name)
{
    // Provide minimal valid arguments for the Recipe constructor
    return RecipeApp::Recipe(id, name, {}, {}, 0, RecipeApp::Difficulty::Easy, "");
}

int main()
{
    std::cout << "--- Running RestaurantManager Tests ---" << std::endl;

    // Test Case 1: Add Restaurant and Get All
    bool test1_success = true;
    bool test2_success = true;
    bool test3_success = true;
    bool test4_success = true;
    bool test5_success = true;
    bool test6_success = true;
    // All test success flags are now declared and initialized here at the top of main.
    try
    {
        RecipeApp::Persistence::JsonRestaurantRepository repo_rm1("test_case1_restaurants.json");
        // Ensure a clean state for the test by not loading, or explicitly clearing if needed.
        // repo_rm1.saveAll(); // Clears the file if m_restaurants is empty
        RecipeApp::RestaurantManager rm1(repo_rm1);
        RecipeApp::Restaurant r1_in(0, "Cafe One", "Addr 1", "Cont 1", "Hours 1"); // ID will be assigned by manager
        RecipeApp::Restaurant r2_in(0, "Cafe Two", "Addr 2", "Cont 2", "Hours 2");

        assert(rm1.addRestaurant(r1_in));
        assert(rm1.addRestaurant(r2_in));

        const auto &allRestaurants = rm1.getAllRestaurants();
        assert(allRestaurants.size() == 2);

        // Check if IDs were assigned correctly (assuming sequential IDs starting from 1)
        bool found1 = false, found2 = false;
        for (const auto &r : allRestaurants)
        {
            if (r.getRestaurantId() == 1 && r.getName() == "Cafe One")
                found1 = true;
            if (r.getRestaurantId() == 2 && r.getName() == "Cafe Two")
                found2 = true;
        }
        assert(found1 && found2);
        assert(rm1.getNextRestaurantId() == 3); // Next ID should be 3
    }
    catch (...)
    {
        test1_success = false;
    }
    printTestResult("Add Restaurant and Get All", test1_success);

    // Test Case 2: Find Restaurant By ID
    try
    {
        RecipeApp::Persistence::JsonRestaurantRepository repo_rm2("test_case2_restaurants.json");
        RecipeApp::RestaurantManager rm2(repo_rm2);
        RecipeApp::Restaurant r1_in(0, "FindMe", "Addr Find", "Cont Find", "Hours Find");
        rm2.addRestaurant(r1_in); // ID should become 1

        std::optional<RecipeApp::Restaurant> foundOpt = rm2.findRestaurantById(1);
        assert(foundOpt.has_value());
        assert(foundOpt.value().getName() == "FindMe");

        std::optional<RecipeApp::Restaurant> notFoundOpt = rm2.findRestaurantById(99);
        assert(!notFoundOpt.has_value());
    }
    catch (...)
    {
        test2_success = false;
    }
    printTestResult("Find Restaurant By ID", test2_success);

    // Test Case 3: Find Restaurant By Name (Exact and Fuzzy)
    try
    {
        RecipeApp::Persistence::JsonRestaurantRepository repo_rm3("test_case3_restaurants.json");
        RecipeApp::RestaurantManager rm3(repo_rm3);
        RecipeApp::Restaurant r1_in(0, "Burger Joint", "Addr B", "Cont B", "Hours B");
        RecipeApp::Restaurant r2_in(0, "Super Burger", "Addr SB", "Cont SB", "Hours SB");
        RecipeApp::Restaurant r3_in(0, "Pizza Place", "Addr P", "Cont P", "Hours P");
        rm3.addRestaurant(r1_in);
        rm3.addRestaurant(r2_in);
        rm3.addRestaurant(r3_in);

        std::vector<RecipeApp::Restaurant> results;

        // Exact match
        results = rm3.findRestaurantByName("Burger Joint", false);
        assert(results.size() == 1);
        assert(results[0].getName() == "Burger Joint");

        // Fuzzy match
        results = rm3.findRestaurantByName("burger", true); // Case-insensitive fuzzy
        assert(results.size() == 2);                        // "Burger Joint", "Super Burger"

        // Fuzzy match no results
        results = rm3.findRestaurantByName("Taco", true);
        assert(results.empty());

        // Exact match no results
        results = rm3.findRestaurantByName("Pizza Joint", false);
        assert(results.empty());
    }
    catch (...)
    {
        test3_success = false;
    }
    printTestResult("Find Restaurant By Name", test3_success);

    // Test Case 4: Get Featured Recipes
    try
    {
        RecipeApp::Persistence::JsonRecipeRepository repo_recipe_tc4("test_case4_recipes.json");
        RecipeApp::RecipeManager recipeMgr(repo_recipe_tc4);
        RecipeApp::Persistence::JsonRestaurantRepository repo_restaurant_tc4("test_case4_restaurants.json");
        RecipeApp::RestaurantManager restaurantMgr(repo_restaurant_tc4);

        // Add recipes
        RecipeApp::Recipe recipe_in1 = createDummyRecipe(0, "Cheeseburger"); // ID 0, will be assigned by manager
        RecipeApp::Recipe recipe_in2 = createDummyRecipe(0, "Fries");
        RecipeApp::Recipe recipe_in3 = createDummyRecipe(0, "Milkshake");

        int assigned_id1 = recipeMgr.addRecipe(recipe_in1); // Capture assigned ID (should be 1)
        int assigned_id2 = recipeMgr.addRecipe(recipe_in2); // Capture assigned ID (should be 2)
        int assigned_id3 = recipeMgr.addRecipe(recipe_in3); // Capture assigned ID (should be 3)

        assert(assigned_id1 == 1);
        assert(assigned_id2 == 2);
        assert(assigned_id3 == 3);

        // Add restaurant and feature recipes using assigned IDs
        RecipeApp::Restaurant r_feat_in(0, "Featured Food", "Addr Feat", "Cont Feat", "Hours Feat");
        r_feat_in.addFeaturedRecipe(assigned_id1); // Feature Cheeseburger (ID 1)
        r_feat_in.addFeaturedRecipe(assigned_id3); // Feature Milkshake (ID 3)
        restaurantMgr.addRestaurant(r_feat_in);    // Restaurant ID becomes 1

        std::vector<RecipeApp::Recipe> featuredResults;
        // Get featured recipes for restaurant with ID 1
        featuredResults = restaurantMgr.getFeaturedRecipes(1, recipeMgr);

        assert(featuredResults.size() == 2);
        bool foundR_assigned1 = false, foundR_assigned3 = false;
        for (const auto &r : featuredResults)
        {
            if (r.getRecipeId() == assigned_id1 && r.getName() == "Cheeseburger")
                foundR_assigned1 = true;
            if (r.getRecipeId() == assigned_id3 && r.getName() == "Milkshake")
                foundR_assigned3 = true;
        }
        assert(foundR_assigned1 && foundR_assigned3);

        // Test for restaurant not found
        featuredResults = restaurantMgr.getFeaturedRecipes(99, recipeMgr);
        assert(featuredResults.empty());
    }
    catch (...)
    {
        test4_success = false;
    }
    printTestResult("Get Featured Recipes", test4_success);
    // Test Case 5: Delete Restaurant
    try
    {
        RecipeApp::Persistence::JsonRestaurantRepository repo_rm5("test_case5_restaurants.json");
        RecipeApp::RestaurantManager rm5(repo_rm5);
        RecipeApp::Restaurant r1_del(0, "ToDelete1", "Addr Del1", "Cont Del1", "Hours Del1");
        RecipeApp::Restaurant r2_del(0, "ToKeep", "Addr Keep", "Cont Keep", "Hours Keep");
        rm5.addRestaurant(r1_del); // ID will be 1
        rm5.addRestaurant(r2_del); // ID will be 2

        assert(rm5.getAllRestaurants().size() == 2);

        // Delete existing restaurant
        assert(rm5.deleteRestaurant(1)); // Delete "ToDelete1"
        assert(rm5.getAllRestaurants().size() == 1);
        std::optional<RecipeApp::Restaurant> keptOpt = rm5.findRestaurantById(2);
        assert(keptOpt.has_value() && keptOpt.value().getName() == "ToKeep");
        assert(rm5.findRestaurantById(1) == std::nullopt);

        // Try to delete non-existent restaurant
        assert(!rm5.deleteRestaurant(99));
        assert(rm5.getAllRestaurants().size() == 1);
        // Delete remaining restaurant from Test Case 5
        assert(rm5.deleteRestaurant(2));
        assert(rm5.getAllRestaurants().empty());
    } // This is the end of Test Case 5's try block
    catch (...)
    {
        test5_success = false;
    }
    printTestResult("Delete Restaurant", test5_success);

    // Test Case 6: Update Restaurant (Now correctly un-nested)
    try
    {
        RecipeApp::Persistence::JsonRestaurantRepository repo_rm6("test_case6_restaurants.json");
        RecipeApp::RestaurantManager rm6(repo_rm6);
        RecipeApp::Restaurant r_orig1(0, "Original One", "Addr O1", "Cont O1", "Hours O1");
        RecipeApp::Restaurant r_orig2(0, "Original Two", "Addr O2", "Cont O2", "Hours O2");
        rm6.addRestaurant(r_orig1); // ID 1
        rm6.addRestaurant(r_orig2); // ID 2

        // Successful update
        RecipeApp::Restaurant r_updated1(1, "Updated One", "Addr U1", "Cont U1", "Hours U1");
        r_updated1.addFeaturedRecipe(501);
        assert(rm6.updateRestaurant(r_updated1));
        std::optional<RecipeApp::Restaurant> p_updated1Opt = rm6.findRestaurantById(1);
        assert(p_updated1Opt.has_value());
        assert(p_updated1Opt.value().getName() == "Updated One");
        assert(p_updated1Opt.value().getAddress() == "Addr U1");
        assert(!p_updated1Opt.value().getFeaturedRecipeIds().empty() && p_updated1Opt.value().getFeaturedRecipeIds()[0] == 501);

        // Try to update non-existent restaurant
        RecipeApp::Restaurant r_non_existent(99, "Non Existent", "Addr NE", "Cont NE", "Hours NE");
        assert(!rm6.updateRestaurant(r_non_existent));

        // Try to update with a name that conflicts with another existing restaurant
        RecipeApp::Restaurant r_conflict(1, "Original Two", "Addr Conflict", "Cont Conflict", "Hours Conflict"); // Try to rename R1 to "Original Two"
        assert(!rm6.updateRestaurant(r_conflict));
        assert(p_updated1Opt.value().getName() == "Updated One"); // R1 should not have changed

        // Update name to something unique (or same as before) - should succeed
        RecipeApp::Restaurant r_rename_ok(1, "Unique Name For One", "Addr U1", "Cont U1", "Hours U1");
        assert(rm6.updateRestaurant(r_rename_ok));
        // Re-fetch to check the update
        std::optional<RecipeApp::Restaurant> p_after_rename_opt = rm6.findRestaurantById(1);
        assert(p_after_rename_opt.has_value());
        assert(p_after_rename_opt.value().getName() == "Unique Name For One");

        // Update R2 without name change
        RecipeApp::Restaurant r_updated2(2, "Original Two", "Addr U2", "Cont U2", "Hours U2");
        assert(rm6.updateRestaurant(r_updated2));
        std::optional<RecipeApp::Restaurant> p_updated2Opt = rm6.findRestaurantById(2);
        assert(p_updated2Opt.has_value() && p_updated2Opt.value().getAddress() == "Addr U2");
    }
    catch (...)
    {
        test6_success = false;
    }
    printTestResult("Update Restaurant", test6_success);

    std::cout << "--- RestaurantManager Tests Finished ---" << std::endl;

    return (test1_success && test2_success && test3_success && test4_success && test5_success && test6_success) ? 0 : 1;
}