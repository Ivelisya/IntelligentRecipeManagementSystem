#include "gtest/gtest.h"
#include "persistence/JsonRecipeRepository.h"
#include "domain/recipe/Recipe.h"
#include "json.hpp"

#include <fstream>
#include <cstdio> // For std::remove
#include <vector>
#include <optional>

using json = nlohmann::json;
using namespace RecipeApp;
using namespace RecipeApp::Persistence;
using RecipeApp::Domain::Recipe::RecipeRepository;

// Helper to create a recipe
Recipe createSimpleRecipe(int id, const std::string &name, const std::string &cuisine = "TestCuisine")
{
    return Recipe(id, name, {{"i1", "q1"}}, {"s1"}, 10, Difficulty::Easy, cuisine);
}

class JsonRecipeRepositoryTest : public ::testing::Test
{
protected:
    const std::string testFilePath = "test_recipes.json";

    void SetUp() override
    {
        // Ensure no test file from previous runs
        std::remove(testFilePath.c_str());
    }

    void TearDown() override
    {
        // Clean up the test file
        std::remove(testFilePath.c_str());
    }

    // Helper to write raw JSON content to the test file
    void writeJsonToFile(const json &jsonData)
    {
        std::ofstream outFile(testFilePath);
        outFile << jsonData.dump(2);
        outFile.close();
    }
};

TEST_F(JsonRecipeRepositoryTest, Constructor)
{
    ASSERT_NO_THROW(JsonRecipeRepository repo(testFilePath));
}

TEST_F(JsonRecipeRepositoryTest, LoadNonExistentFile)
{
    JsonRecipeRepository repo(testFilePath);
    ASSERT_TRUE(repo.load()); // Should succeed and start with an empty list
    EXPECT_EQ(repo.findAll().size(), 0);
    EXPECT_EQ(repo.getNextId(), 1);
}

TEST_F(JsonRecipeRepositoryTest, LoadEmptyJsonFile)
{
    json emptyData = json::object(); // e.g. {} or {"recipes": []}
    emptyData["recipes"] = json::array();
    writeJsonToFile(emptyData);

    JsonRecipeRepository repo(testFilePath);
    ASSERT_TRUE(repo.load());
    EXPECT_EQ(repo.findAll().size(), 0);
    EXPECT_EQ(repo.getNextId(), 1);
}

TEST_F(JsonRecipeRepositoryTest, LoadValidRecipes)
{
    json testData;
    json recipesArray = json::array();
    recipesArray.push_back(json(createSimpleRecipe(1, "Recipe 1")));
    recipesArray.push_back(json(createSimpleRecipe(2, "Recipe 2")));
    testData["recipes"] = recipesArray;
    writeJsonToFile(testData);

    JsonRecipeRepository repo(testFilePath);
    ASSERT_TRUE(repo.load());
    ASSERT_EQ(repo.findAll().size(), 2);
    EXPECT_EQ(repo.findById(1).value().getName(), "Recipe 1");
    EXPECT_EQ(repo.findById(2).value().getName(), "Recipe 2");
    EXPECT_EQ(repo.getNextId(), 3); // Max ID was 2, so next is 3
}

TEST_F(JsonRecipeRepositoryTest, LoadInvalidRecipeDataInFile)
{
    json testData;
    json recipesArray = json::array();
    json validRecipeJson = createSimpleRecipe(1, "Valid Recipe 1");
    json invalidRecipeJson = {
        // Missing 'name', 'cuisine', etc. which from_json now requires
        {"id", 2}
        // name, cuisine, cookingTime, difficulty, ingredients, steps are missing
    };
    json recipeWithBadIdJson = createSimpleRecipe(0, "Bad ID Recipe");

    recipesArray.push_back(validRecipeJson);
    recipesArray.push_back(invalidRecipeJson);   // This should cause from_json to throw
    recipesArray.push_back(recipeWithBadIdJson); // This should be skipped by load logic

    testData["recipes"] = recipesArray;
    writeJsonToFile(testData);

    JsonRecipeRepository repo(testFilePath);
    // load() catches exceptions from from_json and logs, then continues if possible.
    // The behavior of load is to skip recipes that fail deserialization or have invalid IDs.
    // It should still return true if the file itself was parsed, but log errors.
    ASSERT_TRUE(repo.load()); // File parsing itself is okay

    // Only the valid recipe should be loaded.
    // The one with missing fields will throw during its from_json, caught by repo.load().
    // The one with ID 0 will be skipped by repo.load()'s own check.
    ASSERT_EQ(repo.findAll().size(), 1);
    EXPECT_EQ(repo.findById(1).value().getName(), "Valid Recipe 1");
    EXPECT_EQ(repo.getNextId(), 2); // Max valid ID loaded was 1
}

TEST_F(JsonRecipeRepositoryTest, SaveAllAndReload)
{
    JsonRecipeRepository repo(testFilePath);
    Recipe r1 = createSimpleRecipe(0, "First Save"); // ID 0 to get new ID
    Recipe r2 = createSimpleRecipe(0, "Second Save");

    int id1 = repo.save(r1);
    int id2 = repo.save(r2);

    EXPECT_GT(id1, 0);
    EXPECT_GT(id2, 0);
    EXPECT_NE(id1, id2);

    // saveAll is called by save. Now reload in a new repo instance.
    JsonRecipeRepository repo2(testFilePath);
    ASSERT_TRUE(repo2.load());
    ASSERT_EQ(repo2.findAll().size(), 2);
    EXPECT_TRUE(repo2.findById(id1).has_value());
    EXPECT_TRUE(repo2.findById(id2).has_value());
    EXPECT_EQ(repo2.findById(id1).value().getName(), "First Save");
}

TEST_F(JsonRecipeRepositoryTest, SaveNewRecipe)
{
    JsonRecipeRepository repo(testFilePath);
    Recipe newRecipe = createSimpleRecipe(0, "New Dish"); // ID 0 means generate new

    int newId = repo.save(newRecipe);
    EXPECT_EQ(newId, 1); // First ID generated
    EXPECT_EQ(repo.getNextId(), 2);

    std::optional<Recipe> saved = repo.findById(newId);
    ASSERT_TRUE(saved.has_value());
    EXPECT_EQ(saved.value().getName(), "New Dish");

    Recipe anotherNew = createSimpleRecipe(0, "Another Dish");
    int anotherId = repo.save(anotherNew);
    EXPECT_EQ(anotherId, 2);
    EXPECT_EQ(repo.getNextId(), 3);
}

TEST_F(JsonRecipeRepositoryTest, UpdateExistingRecipe)
{
    JsonRecipeRepository repo(testFilePath);
    Recipe r1 = createSimpleRecipe(0, "Original Name");
    int id1 = repo.save(r1); // Gets ID 1

    Recipe recipeToUpdate = repo.findById(id1).value();
    recipeToUpdate.setName("Updated Name");
    recipeToUpdate.setCookingTime(100);

    int updatedId = repo.save(recipeToUpdate); // Should update existing
    EXPECT_EQ(updatedId, id1);                 // ID should remain the same
    EXPECT_EQ(repo.findAll().size(), 1);       // Still only one recipe

    std::optional<Recipe> fetched = repo.findById(id1);
    ASSERT_TRUE(fetched.has_value());
    EXPECT_EQ(fetched.value().getName(), "Updated Name");
    EXPECT_EQ(fetched.value().getCookingTime(), 100);
    EXPECT_EQ(repo.getNextId(), 2); // Next ID should still be 2
}

TEST_F(JsonRecipeRepositoryTest, FindById)
{
    JsonRecipeRepository repo(testFilePath);
    repo.save(createSimpleRecipe(0, "R1")); // ID 1
    repo.save(createSimpleRecipe(0, "R2")); // ID 2

    ASSERT_TRUE(repo.findById(1).has_value());
    EXPECT_EQ(repo.findById(1).value().getName(), "R1");
    ASSERT_TRUE(repo.findById(2).has_value());
    EXPECT_EQ(repo.findById(2).value().getName(), "R2");
    ASSERT_FALSE(repo.findById(3).has_value());
}

TEST_F(JsonRecipeRepositoryTest, FindByName)
{
    JsonRecipeRepository repo(testFilePath);
    repo.save(createSimpleRecipe(0, "Apple Pie", "Dessert"));
    repo.save(createSimpleRecipe(0, "Apple Crumble", "Dessert"));
    repo.save(createSimpleRecipe(0, "Beef Stew", "Main"));

    // Exact match
    CustomLinkedList<Recipe> pies = repo.findByName("Apple Pie");
    ASSERT_EQ(pies.getSize(), 1);
    EXPECT_EQ(pies.getAtIndex(0).getName(), "Apple Pie");

    // Partial match
    CustomLinkedList<Recipe> apples = repo.findByName("Apple", true);
    ASSERT_EQ(apples.getSize(), 2); // Case insensitive search
    CustomLinkedList<Recipe> lower_apples = repo.findByName("apple", true);
    ASSERT_EQ(lower_apples.getSize(), 2); // Should match "Apple" regardless of case

    // No match
    CustomLinkedList<Recipe> noMatch = repo.findByName("NonExistent");
    ASSERT_EQ(noMatch.getSize(), 0);
}

TEST_F(JsonRecipeRepositoryTest, FindAll)
{
    JsonRecipeRepository repo(testFilePath);
    EXPECT_EQ(repo.findAll().size(), 0);

    repo.save(createSimpleRecipe(0, "R1"));
    repo.save(createSimpleRecipe(0, "R2"));
    std::vector<Recipe> all = repo.findAll();
    ASSERT_EQ(all.size(), 2);
}

TEST_F(JsonRecipeRepositoryTest, RemoveRecipe)
{
    JsonRecipeRepository repo(testFilePath);
    int id1 = repo.save(createSimpleRecipe(0, "To Remove"));
    repo.save(createSimpleRecipe(0, "To Keep"));

    ASSERT_EQ(repo.findAll().size(), 2);
    ASSERT_TRUE(repo.remove(id1));
    ASSERT_EQ(repo.findAll().size(), 1);
    ASSERT_FALSE(repo.findById(id1).has_value());
    ASSERT_TRUE(repo.findById(id1 + 1).has_value()); // Assuming next ID was id1+1

    ASSERT_FALSE(repo.remove(999)); // Remove non-existent
    ASSERT_EQ(repo.findAll().size(), 1);
}

TEST_F(JsonRecipeRepositoryTest, GetAndSetNextId)
{
    JsonRecipeRepository repo(testFilePath);
    EXPECT_EQ(repo.getNextId(), 1);
    repo.setNextId(100);
    EXPECT_EQ(repo.getNextId(), 100);

    int id = repo.save(createSimpleRecipe(0, "Test"));
    EXPECT_EQ(id, 100);
    EXPECT_EQ(repo.getNextId(), 101);
}

// Test saving a recipe with a pre-assigned ID that is not yet in the list
// This simulates loading from a persistence layer where IDs might already exist
// and then adding a new recipe that also has a pre-assigned ID (e.g. from another system)
TEST_F(JsonRecipeRepositoryTest, SaveRecipeWithPreAssignedIdNotInList)
{
    JsonRecipeRepository repo(testFilePath); // Empty repo, m_nextId = 1

    Recipe r_pre_assigned = createSimpleRecipe(5, "Pre-assigned ID 5");
    int saved_id = repo.save(r_pre_assigned);

    EXPECT_EQ(saved_id, 5); // Should use the provided ID
    ASSERT_TRUE(repo.findById(5).has_value());
    EXPECT_EQ(repo.findById(5).value().getName(), "Pre-assigned ID 5");
    EXPECT_EQ(repo.getNextId(), 6); // m_nextId should be updated to be > max existing ID

    // Save another new recipe, should get ID 6
    Recipe r_new = createSimpleRecipe(0, "New after pre-assigned");
    int new_id = repo.save(r_new);
    EXPECT_EQ(new_id, 6);
    EXPECT_EQ(repo.getNextId(), 7);
}

TEST_F(JsonRecipeRepositoryTest, SaveRecipeWithPreAssignedIdAlreadyInList_ShouldUpdate)
{
    JsonRecipeRepository repo(testFilePath);
    repo.save(createSimpleRecipe(0, "Initial Recipe with ID 1")); // Gets ID 1

    Recipe r_update = createSimpleRecipe(1, "Updated Recipe with ID 1");
    r_update.setCookingTime(99);

    int saved_id = repo.save(r_update);
    EXPECT_EQ(saved_id, 1);              // Should be an update, ID remains 1
    ASSERT_EQ(repo.findAll().size(), 1); // Still one recipe

    std::optional<Recipe> fetched = repo.findById(1);
    ASSERT_TRUE(fetched.has_value());
    EXPECT_EQ(fetched.value().getName(), "Updated Recipe with ID 1");
    EXPECT_EQ(fetched.value().getCookingTime(), 99);
    EXPECT_EQ(repo.getNextId(), 2); // Next available ID should be 2
}