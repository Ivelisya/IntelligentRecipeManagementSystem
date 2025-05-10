#include "gtest/gtest.h"
#include "logic/encyclopedia/RecipeEncyclopediaManager.h"
#include "domain/recipe/Recipe.h" // For RecipeApp::Recipe and RecipeApp::RecipeBuilder
#include <vector>
#include <string>
#include <optional>
#include <fstream> // Required for std::ofstream

// Helper function to create a dummy recipe for testing
RecipeApp::Recipe createDummyRecipe(int id, const std::string& name, const std::vector<std::string>& tags = {}) {
    return RecipeApp::Recipe::builder(id, name)
        .withCookingTime(30)
        .withDifficulty(RecipeApp::Difficulty::Easy)
        .withIngredients({{"Flour", "1 cup"}, {"Sugar", "0.5 cup"}})
        .withSteps({"Mix ingredients", "Bake"})
        .withTags(tags)
        .build();
}

class RecipeEncyclopediaManagerTest : public ::testing::Test {
protected:
    RecipeApp::Logic::Encyclopedia::RecipeEncyclopediaManager manager;
    std::string testRecipesJsonPath = "test_encyclopedia_recipes.json";

    void SetUp() override {
        // Create a dummy JSON file for tests that require loading
        // For getRecipeById, we can directly populate encyclopediaRecipes
        // or use a mock repository if the manager used one (it doesn't directly)
        
        // For simplicity in testing getRecipeById and searchRecipes directly without file IO for these specific tests,
        // we can manually add recipes to the manager if it had a public method to do so,
        // or we test loadRecipes separately and then test other methods.
        // Since RecipeEncyclopediaManager loads from file in its current design,
        // and doesn't have a direct "addRecipe" method for the encyclopedia,
        // we'll focus on testing its state after loading or by directly manipulating its internal state if possible (not ideal).
        // A better approach for unit testing would be to allow injecting recipes or a recipe source.

        // For now, let's assume loadRecipes works and we test based on a known state.
        // Or, we can create a temporary file for loadRecipes.
        // Let's create a temporary file for loading.
        std::ofstream outfile(testRecipesJsonPath);
        outfile << R"([
            {
                "id": 101, "name": "Apple Pie", "cookingTime": 60, "difficulty": "Medium",
                "ingredients": [{"name": "Apple", "quantity": "3"}, {"name": "Crust", "quantity": "1"}],
                "steps": ["Prepare apples", "Bake pie"], "tags": ["dessert", "fruit"]
            },
            {
                "id": 102, "name": "Tomato Soup", "cookingTime": 30, "difficulty": "Easy",
                "ingredients": [{"name": "Tomato", "quantity": "5"}, {"name": "Broth", "quantity": "2 cups"}],
                "steps": ["Simmer tomatoes", "Blend soup"], "tags": ["soup", "vegetarian"]
            },
            {
                "id": 103, "name": "Grilled Chicken", "cookingTime": 45, "difficulty": "Medium",
                "ingredients": [{"name": "Chicken Breast", "quantity": "2"}, {"name": "Spice Mix", "quantity": "1 tbsp"}],
                "steps": ["Marinate chicken", "Grill chicken"], "tags": ["main course", "grill"]
            }
        ])";
        outfile.close();
        ASSERT_TRUE(manager.loadRecipes(testRecipesJsonPath));
    }

    void TearDown() override {
        std::remove(testRecipesJsonPath.c_str());
    }
};

TEST_F(RecipeEncyclopediaManagerTest, GetRecipeByIdExisting) {
    std::optional<RecipeApp::Recipe> recipe = manager.getRecipeById(101);
    ASSERT_TRUE(recipe.has_value());
    EXPECT_EQ(recipe->getRecipeId(), 101);
    EXPECT_EQ(recipe->getName(), "Apple Pie");

    recipe = manager.getRecipeById(102);
    ASSERT_TRUE(recipe.has_value());
    EXPECT_EQ(recipe->getRecipeId(), 102);
    EXPECT_EQ(recipe->getName(), "Tomato Soup");
}

TEST_F(RecipeEncyclopediaManagerTest, GetRecipeByIdNonExisting) {
    std::optional<RecipeApp::Recipe> recipe = manager.getRecipeById(999);
    ASSERT_FALSE(recipe.has_value());
}

TEST_F(RecipeEncyclopediaManagerTest, GetRecipeByIdWithEmptyEncyclopedia) {
    RecipeApp::Logic::Encyclopedia::RecipeEncyclopediaManager emptyManager;
    // Create an empty temp file
    std::ofstream outfile_empty("empty_recipes.json");
    outfile_empty << "[]";
    outfile_empty.close();
    ASSERT_TRUE(emptyManager.loadRecipes("empty_recipes.json"));
    std::remove("empty_recipes.json");

    std::optional<RecipeApp::Recipe> recipe = emptyManager.getRecipeById(101);
    ASSERT_FALSE(recipe.has_value());
}

TEST_F(RecipeEncyclopediaManagerTest, SearchRecipesByName) {
    std::vector<RecipeApp::Recipe> results = manager.searchRecipes("Apple Pie");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].getName(), "Apple Pie");

    results = manager.searchRecipes("pie"); // Case-insensitive partial match
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].getName(), "Apple Pie");
}

TEST_F(RecipeEncyclopediaManagerTest, SearchRecipesByIngredient) {
    std::vector<RecipeApp::Recipe> results = manager.searchRecipes("Tomato");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].getName(), "Tomato Soup");
}

TEST_F(RecipeEncyclopediaManagerTest, SearchRecipesByTag) {
    std::vector<RecipeApp::Recipe> results = manager.searchRecipes("dessert");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].getName(), "Apple Pie");

    results = manager.searchRecipes("grill");
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].getName(), "Grilled Chicken");
}

TEST_F(RecipeEncyclopediaManagerTest, SearchRecipesNoMatch) {
    std::vector<RecipeApp::Recipe> results = manager.searchRecipes("NonExistentRecipe");
    ASSERT_TRUE(results.empty());
}

TEST_F(RecipeEncyclopediaManagerTest, SearchRecipesEmptyTerm) {
    // As per current implementation, empty search term returns all recipes
    std::vector<RecipeApp::Recipe> results = manager.searchRecipes("");
    EXPECT_EQ(results.size(), 3); // All recipes loaded in SetUp
}

TEST_F(RecipeEncyclopediaManagerTest, LoadRecipesFileNotFound) {
    RecipeApp::Logic::Encyclopedia::RecipeEncyclopediaManager newManager;
    EXPECT_FALSE(newManager.loadRecipes("non_existent_file.json"));
}

TEST_F(RecipeEncyclopediaManagerTest, LoadRecipesMalformedJson) {
    RecipeApp::Logic::Encyclopedia::RecipeEncyclopediaManager newManager;
    std::string malformedJsonPath = "malformed_recipes.json";
    std::ofstream outfile(malformedJsonPath);
    outfile << R"([ { "id": 201, "name": "Bad Pie", )"; // Incomplete JSON
    outfile.close();
    EXPECT_FALSE(newManager.loadRecipes(malformedJsonPath));
    std::remove(malformedJsonPath.c_str());
}

TEST_F(RecipeEncyclopediaManagerTest, GetAllRecipes) {
    const auto& allRecipes = manager.getAllRecipes();
    ASSERT_EQ(allRecipes.size(), 3);
    // Check if one of the known recipes is present
    bool found = false;
    for(const auto& recipe : allRecipes) {
        if (recipe.getRecipeId() == 103 && recipe.getName() == "Grilled Chicken") {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}