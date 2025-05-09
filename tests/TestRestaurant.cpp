#include "../src/domain/restaurant/Restaurant.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

// Helper function to print test results
void printTestResult(const std::string &testName, bool success)
{
    std::cout << "Test " << testName << ": " << (success ? "PASSED" : "FAILED") << std::endl;
}

int main()
{
    std::cout << "--- Running Restaurant Tests ---" << std::endl;

    // Test Case 1: Constructor and Getters
    bool test1_success = true;
    try
    {
        RecipeApp::Restaurant r1(1, "Test Cafe", "123 Main St", "555-1234", "9am-5pm");
        assert(r1.getRestaurantId() == 1);
        assert(r1.getName() == "Test Cafe");
        assert(r1.getAddress() == "123 Main St");
        assert(r1.getContact() == "555-1234");
        assert(r1.getOpeningHours() == "9am-5pm");
        assert(r1.getFeaturedRecipeIds().empty()); // Initially empty
    }
    catch (...)
    {
        test1_success = false;
    }
    printTestResult("Constructor and Getters", test1_success);

    // Test Case 2: Setters
    bool test2_success = true;
    try
    {
        RecipeApp::Restaurant r2(2, "Old Name", "Old Address", "Old Contact", "Old Hours");
        r2.setName("New Cafe");
        r2.setAddress("456 Oak Ave");
        r2.setContact("555-5678");
        r2.setOpeningHours("10am-6pm");
        assert(r2.getName() == "New Cafe");
        assert(r2.getAddress() == "456 Oak Ave");
        assert(r2.getContact() == "555-5678");
        assert(r2.getOpeningHours() == "10am-6pm");
    }
    catch (...)
    {
        test2_success = false;
    }
    printTestResult("Setters", test2_success);

    // Test Case 3: Add Featured Recipe IDs
    bool test3_success = true;
    try
    {
        RecipeApp::Restaurant r3(3, "Recipe Hub", "789 Pine Ln", "555-9012", "8am-8pm");
        r3.addFeaturedRecipe(101);
        r3.addFeaturedRecipe(102);
        r3.addFeaturedRecipe(101); // Add duplicate, should be ignored
        const std::vector<int> &ids = r3.getFeaturedRecipeIds();
        assert(ids.size() == 2);
        assert(ids[0] == 101);
        assert(ids[1] == 102);
    }
    catch (...)
    {
        test3_success = false;
    }
    printTestResult("Add Featured Recipe IDs", test3_success);

    // Test Case 4: Remove Featured Recipe IDs
    bool test4_success = true;
    try
    {
        RecipeApp::Restaurant r4(4, "Food Spot", "321 Elm St", "555-3456", "11am-9pm");
        r4.addFeaturedRecipe(201);
        r4.addFeaturedRecipe(202);
        r4.addFeaturedRecipe(203);
        r4.removeFeaturedRecipe(202); // Remove middle
        r4.removeFeaturedRecipe(204); // Remove non-existent
        r4.removeFeaturedRecipe(201); // Remove first
        const std::vector<int> &ids = r4.getFeaturedRecipeIds();
        assert(ids.size() == 1);
        assert(ids[0] == 203);
        r4.removeFeaturedRecipe(203); // Remove last
        assert(r4.getFeaturedRecipeIds().empty());
    }
    catch (...)
    {
        test4_success = false;
    }
    printTestResult("Remove Featured Recipe IDs", test4_success);

    // Test Case 5: Display (Manual Check) - Optional
    // std::cout << "\n--- Manual Check for Display ---" << std::endl;
    // RecipeApp::Restaurant r5(5, "Display Test", "Addr", "Cont", "Hours");
    // r5.addFeaturedRecipe(301);
    // r5.addFeaturedRecipe(302);
    // r5.displayRestaurantDetails();
    // std::cout << "------------------------------" << std::endl;

    std::cout << "--- Restaurant Tests Finished ---" << std::endl;

    // Indicate overall success (simple approach)
    return (test1_success && test2_success && test3_success && test4_success) ? 0 : 1;
}