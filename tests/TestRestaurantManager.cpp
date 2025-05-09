#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "../src/core/CustomLinkedList.h"
#include "../src/domain/recipe/Recipe.h"  // Needed for Recipe objects
#include "../src/domain/restaurant/Restaurant.h"
#include "../src/logic/recipe/RecipeManager.h"  // Needed for getFeaturedRecipes test
#include "../src/logic/restaurant/RestaurantManager.h"
#include "../src/persistence/JsonRecipeRepository.h"      // Added for testing
#include "../src/persistence/JsonRestaurantRepository.h"  // Added for testing

// Helper function from TestRestaurant.cpp (or move to a common test utility
// header)
void printTestResult(const std::string &testName, bool success) {
    std::cout << "Test " << testName << ": " << (success ? "PASSED" : "FAILED")
              << std::endl;
}

// Helper to create a dummy Recipe for testing getFeaturedRecipes
RecipeApp::Recipe createDummyRecipe(int id, const std::string &name) {
    // Provide minimal valid arguments for the Recipe constructor using builder
    return RecipeApp::Recipe::builder(id, name)
        .withIngredients({})
        .withSteps({})
        .withCookingTime(1)  // Ensure positive cooking time if
                             // builder/constructor enforces it
        .withDifficulty(RecipeApp::Difficulty::Easy)
        .withTags({})  // Empty tags
        .build();
}

int main() {
    std::cout << "--- Running RestaurantManager Tests ---" << std::endl;

    // Test Case 1: Add Restaurant and Get All
    bool test1_success = true;
    bool test2_success = true;
    bool test3_success = true;
    bool test4_success = true;
    bool test5_success = true;
    bool test6_success = true;
    // All test success flags are now declared and initialized here at the top
    // of main.
    try {
        // Ensure clean file for Test Case 1
        try {
            if (std::filesystem::exists("test_case1_restaurants.json")) {
                std::filesystem::remove_all("test_case1_restaurants.json");
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error cleaning test_case1_restaurants.json: "
                      << e.what() << " at line " << __LINE__ << std::endl;
            test1_success = false; // Mark test as failed if cleanup fails
        }

        if (test1_success) { // Proceed only if cleanup was successful
            RecipeApp::Persistence::JsonRestaurantRepository repo_rm1(
                ".", "test_case1_restaurants.json");
        // Ensure a clean state for the test by not loading, or explicitly
        // clearing if needed. repo_rm1.saveAll(); // Clears the file if
        // m_restaurants is empty
        RecipeApp::RestaurantManager rm1(repo_rm1);
        RecipeApp::Restaurant r1_in =
            RecipeApp::Restaurant::builder(0, "Cafe One")
                .withAddress("Addr 1")
                .withContact("Cont 1")
                .withOpeningHours("Hours 1")
                .build();
        RecipeApp::Restaurant r2_in =
            RecipeApp::Restaurant::builder(0, "Cafe Two")
                .withAddress("Addr 2")
                .withContact("Cont 2")
                .withOpeningHours("Hours 2")
                .build();

        assert(rm1.addRestaurant(r1_in));
        assert(rm1.addRestaurant(r2_in));

        const auto &allRestaurants = rm1.getAllRestaurants();
        assert(allRestaurants.size() == 2);

        // Check if IDs were assigned correctly (assuming sequential IDs
        // starting from 1)
        bool found1 = false, found2 = false;
        for (const auto &r : allRestaurants) {
            if (r.getRestaurantId() == 1 && r.getName() == "Cafe One")
                found1 = true;
            if (r.getRestaurantId() == 2 && r.getName() == "Cafe Two")
                found2 = true;
        }
        assert(found1 && found2);
        assert(rm1.getNextRestaurantId() == 3);  // Next ID should be 3
    }
    } catch (const std::exception& e) {
        std::cerr << "Test Case 1 caught std::exception: " << e.what() << std::endl;
        test1_success = false;
    } catch (...) {
        std::cerr << "Test Case 1 caught unknown exception." << std::endl;
        test1_success = false;
    }
    printTestResult("Add Restaurant and Get All", test1_success);

    // Test Case 2: Find Restaurant By ID
    try {
        // Ensure clean file for Test Case 2
        try {
            if (std::filesystem::exists("test_case2_restaurants.json")) {
                std::filesystem::remove_all("test_case2_restaurants.json");
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error cleaning test_case2_restaurants.json: "
                      << e.what() << " at line " << __LINE__ << std::endl;
            test2_success = false; // Mark test as failed if cleanup fails
        }

        if (test2_success) { // Proceed only if cleanup was successful
            RecipeApp::Persistence::JsonRestaurantRepository repo_rm2(
                ".", "test_case2_restaurants.json");
        RecipeApp::RestaurantManager rm2(repo_rm2);
        RecipeApp::Restaurant r1_in =
            RecipeApp::Restaurant::builder(0, "FindMe")
                .withAddress("Addr Find")
                .withContact("Cont Find")
                .withOpeningHours("Hours Find")
                .build();
        rm2.addRestaurant(r1_in);  // ID should become 1

        std::optional<RecipeApp::Restaurant> foundOpt =
            rm2.findRestaurantById(1);
        assert(foundOpt.has_value());
        assert(foundOpt.value().getName() == "FindMe");

        std::optional<RecipeApp::Restaurant> notFoundOpt =
            rm2.findRestaurantById(99);
        assert(!notFoundOpt.has_value());
    }
    } catch (const std::exception& e) {
        std::cerr << "Test Case 2 caught std::exception: " << e.what() << std::endl;
        test2_success = false;
    } catch (...) {
        std::cerr << "Test Case 2 caught unknown exception." << std::endl;
        test2_success = false;
    }
    printTestResult("Find Restaurant By ID", test2_success);

    // Test Case 3: Find Restaurant By Name (Exact and Fuzzy)
    try {
        // Ensure clean file for Test Case 3
        try {
            if (std::filesystem::exists("test_case3_restaurants.json")) {
                std::filesystem::remove_all("test_case3_restaurants.json");
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error cleaning test_case3_restaurants.json: "
                      << e.what() << " at line " << __LINE__ << std::endl;
            test3_success = false;
        }

        if (test3_success) {
            RecipeApp::Persistence::JsonRestaurantRepository repo_rm3(
                ".", "test_case3_restaurants.json");
        RecipeApp::RestaurantManager rm3(repo_rm3);
        RecipeApp::Restaurant r1_in =
            RecipeApp::Restaurant::builder(0, "Burger Joint")
                .withAddress("Addr B")
                .withContact("Cont B")
                .withOpeningHours("Hours B")
                .build();
        RecipeApp::Restaurant r2_in =
            RecipeApp::Restaurant::builder(0, "Super Burger")
                .withAddress("Addr SB")
                .withContact("Cont SB")
                .withOpeningHours("Hours SB")
                .build();
        RecipeApp::Restaurant r3_in =
            RecipeApp::Restaurant::builder(0, "Pizza Place")
                .withAddress("Addr P")
                .withContact("Cont P")
                .withOpeningHours("Hours P")
                .build();
        rm3.addRestaurant(r1_in);
        rm3.addRestaurant(r2_in);
        rm3.addRestaurant(r3_in);

        std::vector<RecipeApp::Restaurant> results;

        // Exact match
        results = rm3.findRestaurantByName("Burger Joint", false);
        assert(results.size() == 1);
        assert(results[0].getName() == "Burger Joint");

        // Fuzzy match
        results =
            rm3.findRestaurantByName("burger", true);  // Case-insensitive fuzzy
        assert(results.size() == 2);  // "Burger Joint", "Super Burger"

        // Fuzzy match no results
        results = rm3.findRestaurantByName("Taco", true);
        assert(results.empty());

        // Exact match no results
        results = rm3.findRestaurantByName("Pizza Joint", false);
        assert(results.empty());
    }
    } catch (const std::exception& e) {
        std::cerr << "Test Case 3 caught std::exception: " << e.what() << std::endl;
        test3_success = false;
    } catch (...) {
        std::cerr << "Test Case 3 caught unknown exception." << std::endl;
        test3_success = false;
    }
    printTestResult("Find Restaurant By Name", test3_success);

    // Test Case 4: Get Featured Recipes
    try {
        // Ensure clean files for this test case with detailed error handling
        try {
            if (std::filesystem::exists("test_case4_recipes.json")) {
                std::filesystem::remove_all("test_case4_recipes.json");
            }
        } catch (const std::filesystem::filesystem_error &e) {
            std::cerr
                << "Filesystem error removing_all test_case4_recipes.json: "
                << e.what() << " at line " << __LINE__ << std::endl;
            test4_success = false;
        }

        if (test4_success) {  // Only proceed if previous step was okay
            try {
                if (std::filesystem::exists("test_case4_restaurants.json")) {
                    std::filesystem::remove_all("test_case4_restaurants.json");
                }
            } catch (const std::filesystem::filesystem_error &e) {
                std::cerr << "Filesystem error removing_all "
                             "test_case4_restaurants.json: "
                          << e.what() << " at line " << __LINE__ << std::endl;
                test4_success = false;
            }
        }

        if (test4_success) {  // Only proceed if file cleanup was successful
            RecipeApp::Persistence::JsonRecipeRepository repo_recipe_tc4(
                ".", "test_case4_recipes.json");
            RecipeApp::RecipeManager recipeMgr(repo_recipe_tc4);
            RecipeApp::Persistence::JsonRestaurantRepository
                repo_restaurant_tc4(".", "test_case4_restaurants.json");
            RecipeApp::RestaurantManager restaurantMgr(repo_restaurant_tc4);

            // Add recipes
            RecipeApp::Recipe recipe_in1 = createDummyRecipe(
                0, "Cheeseburger");  // ID 0, will be assigned by manager
            RecipeApp::Recipe recipe_in2 = createDummyRecipe(0, "Fries");
            RecipeApp::Recipe recipe_in3 = createDummyRecipe(0, "Milkshake");

            int assigned_id1 = recipeMgr.addRecipe(
                recipe_in1);  // Capture assigned ID (should be 1)
            int assigned_id2 = recipeMgr.addRecipe(
                recipe_in2);  // Capture assigned ID (should be 2)
            int assigned_id3 = recipeMgr.addRecipe(
                recipe_in3);  // Capture assigned ID (should be 3)

            assert(assigned_id1 == 1);
            assert(assigned_id2 == 2);
            assert(assigned_id3 == 3);

            // Add restaurant and feature recipes using assigned IDs
            RecipeApp::Restaurant r_feat_in =
                RecipeApp::Restaurant::builder(0, "Featured Food")
                    .withAddress("Addr Feat")
                    .withContact("Cont Feat")
                    .withOpeningHours("Hours Feat")
                    .build();
            r_feat_in.addFeaturedRecipe(
                assigned_id1);  // Feature Cheeseburger (ID 1)
            r_feat_in.addFeaturedRecipe(
                assigned_id3);                       // Feature Milkshake (ID 3)
            restaurantMgr.addRestaurant(r_feat_in);  // Restaurant ID becomes 1

            std::vector<RecipeApp::Recipe> featuredResults;
            // Get featured recipes for restaurant with ID 1
            featuredResults = restaurantMgr.getFeaturedRecipes(1, recipeMgr);

            if (featuredResults.size() != 2) {
                std::cerr << "Assertion failed: featuredResults.size() == 2. "
                             "Actual size: "
                          << featuredResults.size() << " at line " << __LINE__
                          << std::endl;
                test4_success = false;
            }
            bool foundR_assigned1 = false, foundR_assigned3 = false;
            for (const auto &r : featuredResults) {
                if (r.getRecipeId() == assigned_id1 &&
                    r.getName() == "Cheeseburger")
                    foundR_assigned1 = true;
                if (r.getRecipeId() == assigned_id3 &&
                    r.getName() == "Milkshake")
                    foundR_assigned3 = true;
            }
            if (!(foundR_assigned1 && foundR_assigned3)) {
                std::cerr << "Assertion failed: foundR_assigned1 && "
                             "foundR_assigned3. "
                          << "foundR_assigned1: " << std::boolalpha
                          << foundR_assigned1
                          << ", foundR_assigned3: " << std::boolalpha
                          << foundR_assigned3 << " at line " << __LINE__
                          << std::endl;
                std::cerr << "Featured results (" << featuredResults.size()
                          << " items):" << std::endl;
                for (const auto &r_debug : featuredResults) {
                    std::cerr << "  ID: " << r_debug.getRecipeId()
                              << ", Name: " << r_debug.getName() << std::endl;
                }
                test4_success = false;
            }

            // Test for restaurant not found
            featuredResults = restaurantMgr.getFeaturedRecipes(99, recipeMgr);
            if (!featuredResults.empty()) {
                std::cerr << "Assertion failed: featuredResults.empty() for "
                             "non-existent restaurant. Actual size: "
                          << featuredResults.size() << " at line " << __LINE__
                          << std::endl;
                test4_success = false;
            }
        }
    }  // This is the new closing brace for the outer try block (line 276)
    catch (const std::exception &e) {  // New catch block starts here
        std::cerr << "Test Case 4 caught std::exception: " << e.what()
                  << std::endl;
        test4_success = false;
    } catch (...) {
        std::cerr << "Test Case 4 caught unknown exception." << std::endl;
        test4_success = false;
    }
    printTestResult("Get Featured Recipes", test4_success);
    // Test Case 5: Delete Restaurant
    try {
        // Ensure clean file for Test Case 5
        try {
            if (std::filesystem::exists("test_case5_restaurants.json")) {
                std::filesystem::remove_all("test_case5_restaurants.json");
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error cleaning test_case5_restaurants.json: "
                      << e.what() << " at line " << __LINE__ << std::endl;
            test5_success = false;
        }

        if (test5_success) {
            RecipeApp::Persistence::JsonRestaurantRepository repo_rm5(
                ".", "test_case5_restaurants.json");
        RecipeApp::RestaurantManager rm5(repo_rm5);
        RecipeApp::Restaurant r1_del =
            RecipeApp::Restaurant::builder(0, "ToDelete1")
                .withAddress("Addr Del1")
                .withContact("Cont Del1")
                .withOpeningHours("Hours Del1")
                .build();
        RecipeApp::Restaurant r2_del =
            RecipeApp::Restaurant::builder(0, "ToKeep")
                .withAddress("Addr Keep")
                .withContact("Cont Keep")
                .withOpeningHours("Hours Keep")
                .build();
        rm5.addRestaurant(r1_del);  // ID will be 1
        rm5.addRestaurant(r2_del);  // ID will be 2

        assert(rm5.getAllRestaurants().size() == 2);

        // Delete existing restaurant
        assert(rm5.deleteRestaurant(1));  // Delete "ToDelete1"
        assert(rm5.getAllRestaurants().size() == 1);
        std::optional<RecipeApp::Restaurant> keptOpt =
            rm5.findRestaurantById(2);
        assert(keptOpt.has_value() && keptOpt.value().getName() == "ToKeep");
        assert(rm5.findRestaurantById(1) == std::nullopt);

        // Try to delete non-existent restaurant
        assert(!rm5.deleteRestaurant(99));
        assert(rm5.getAllRestaurants().size() == 1);
        // Delete remaining restaurant from Test Case 5
        assert(rm5.deleteRestaurant(2));
        assert(rm5.getAllRestaurants().empty());
    }
    } catch (const std::exception& e) { // This was line 387 (end of try block)
        std::cerr << "Test Case 5 caught std::exception: " << e.what() << std::endl;
        test5_success = false;
    } catch (...) {
        std::cerr << "Test Case 5 caught unknown exception." << std::endl;
        test5_success = false;
    }
    printTestResult("Delete Restaurant", test5_success);

    // Test Case 6: Update Restaurant (Now correctly un-nested)
    try {
        // Ensure clean file for Test Case 6
        try {
            if (std::filesystem::exists("test_case6_restaurants.json")) {
                std::filesystem::remove_all("test_case6_restaurants.json");
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error cleaning test_case6_restaurants.json: "
                      << e.what() << " at line " << __LINE__ << std::endl;
            test6_success = false;
        }

        if (test6_success) {
            RecipeApp::Persistence::JsonRestaurantRepository repo_rm6(
                ".", "test_case6_restaurants.json");
        RecipeApp::RestaurantManager rm6(repo_rm6);
        RecipeApp::Restaurant r_orig1 =
            RecipeApp::Restaurant::builder(0, "Original One")
                .withAddress("Addr O1")
                .withContact("Cont O1")
                .withOpeningHours("Hours O1")
                .build();
        RecipeApp::Restaurant r_orig2 =
            RecipeApp::Restaurant::builder(0, "Original Two")
                .withAddress("Addr O2")
                .withContact("Cont O2")
                .withOpeningHours("Hours O2")
                .build();
        rm6.addRestaurant(r_orig1);  // ID 1
        rm6.addRestaurant(r_orig2);  // ID 2

        // Successful update
        RecipeApp::Restaurant r_updated1 =
            RecipeApp::Restaurant::builder(1, "Updated One")
                .withAddress("Addr U1")
                .withContact("Cont U1")
                .withOpeningHours("Hours U1")
                .withFeaturedRecipeIds({501})  // Add featured ID via builder
                .build();
        // r_updated1.addFeaturedRecipe(501); // No longer needed if set by
        // builder
        assert(rm6.updateRestaurant(r_updated1));
        std::optional<RecipeApp::Restaurant> p_updated1Opt =
            rm6.findRestaurantById(1);
        assert(p_updated1Opt.has_value());
        assert(p_updated1Opt.value().getName() == "Updated One");
        assert(p_updated1Opt.value().getAddress() == "Addr U1");
        assert(!p_updated1Opt.value().getFeaturedRecipeIds().empty() &&
               p_updated1Opt.value().getFeaturedRecipeIds()[0] == 501);

        // Try to update non-existent restaurant
        RecipeApp::Restaurant r_non_existent =
            RecipeApp::Restaurant::builder(99, "Non Existent")
                .withAddress("Addr NE")
                .withContact("Cont NE")
                .withOpeningHours("Hours NE")
                .build();
        assert(!rm6.updateRestaurant(r_non_existent));

        // Try to update with a name that conflicts with another existing
        // restaurant
        RecipeApp::Restaurant r_conflict =
            RecipeApp::Restaurant::builder(
                1, "Original Two")  // Try to rename R1 to "Original Two"
                .withAddress("Addr Conflict")
                .withContact("Cont Conflict")
                .withOpeningHours("Hours Conflict")
                .build();
        assert(!rm6.updateRestaurant(r_conflict));
        assert(p_updated1Opt.value().getName() ==
               "Updated One");  // R1 should not have changed

        // Update name to something unique (or same as before) - should
        // succeed
        RecipeApp::Restaurant r_rename_ok =
            RecipeApp::Restaurant::builder(1, "Unique Name For One")
                .withAddress("Addr U1")
                .withContact("Cont U1")
                .withOpeningHours("Hours U1")
                .build();
        assert(rm6.updateRestaurant(r_rename_ok));
        // Re-fetch to check the update
        std::optional<RecipeApp::Restaurant> p_after_rename_opt =
            rm6.findRestaurantById(1);
        assert(p_after_rename_opt.has_value());
        assert(p_after_rename_opt.value().getName() == "Unique Name For One");

        // Update R2 without name change
        RecipeApp::Restaurant r_updated2 =
            RecipeApp::Restaurant::builder(2, "Original Two")
                .withAddress("Addr U2")
                .withContact("Cont U2")
                .withOpeningHours("Hours U2")
                .build();
        assert(rm6.updateRestaurant(r_updated2));
        std::optional<RecipeApp::Restaurant> p_updated2Opt =
            rm6.findRestaurantById(2);
        assert(p_updated2Opt.has_value() &&
               p_updated2Opt.value().getAddress() == "Addr U2");
    }
    } catch (const std::exception& e) {
        std::cerr << "Test Case 6 caught std::exception: " << e.what() << std::endl;
        test6_success = false;
    } catch (...) {
        std::cerr << "Test Case 6 caught unknown exception." << std::endl;
        test6_success = false;
    }
    printTestResult("Update Restaurant", test6_success);

    std::cout << "--- RestaurantManager Tests Finished ---" << std::endl;

    return (test1_success && test2_success && test3_success && test4_success &&
            test5_success && test6_success)
               ? 0
               : 1;
}