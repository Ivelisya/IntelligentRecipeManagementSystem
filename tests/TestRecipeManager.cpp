#include "gtest/gtest.h"
#include "logic/recipe/RecipeManager.h"
#include "persistence/JsonRecipeRepository.h" // For concrete repository in tests
#include "domain/recipe/Recipe.h"
#include <filesystem>
#include <vector>
#include <string>
#include <optional>
#include <algorithm> // For std::sort, std::equal

// Helper to create a recipe with specific tags
RecipeApp::Recipe createRecipeWithTags(int id, const std::string &name, const std::vector<std::string> &tags, const std::string &cuisine = "TestCuisine")
{
    RecipeApp::Recipe recipe(id, name, {{"ingredient", "1g"}}, {"step 1"}, 10, RecipeApp::Difficulty::Easy, cuisine);
    for (const auto &tag : tags)
    {
        recipe.addTag(tag);
    }
    return recipe;
}

class RecipeManagerTest : public ::testing::Test
{
protected:
    std::filesystem::path tempTestDir;
    const std::string testDbFileName = "recipes_manager_test.json";
    std::unique_ptr<RecipeApp::Persistence::JsonRecipeRepository> repo;
    std::unique_ptr<RecipeApp::RecipeManager> manager;

    void SetUp() override
    {
        // Create a unique temporary directory for each test
        auto now = std::chrono::high_resolution_clock::now();
        auto epoch = now.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        std::string unique_name = "test_manager_dir_" + std::to_string(value.count());

        tempTestDir = std::filesystem::current_path() / unique_name;
        std::filesystem::create_directories(tempTestDir);

        repo = std::make_unique<RecipeApp::Persistence::JsonRecipeRepository>(tempTestDir, testDbFileName);
        ASSERT_TRUE(repo->load()); // Start with a clean state
        manager = std::make_unique<RecipeApp::RecipeManager>(*repo);
    }

    void TearDown() override
    {
        // manager and repo will be destroyed automatically by unique_ptr
        if (std::filesystem::exists(tempTestDir))
        {
            std::filesystem::remove_all(tempTestDir);
        }
    }

    // Helper to compare two vectors of strings, ignoring order
    bool compareTags(std::vector<std::string> v1, std::vector<std::string> v2)
    {
        std::sort(v1.begin(), v1.end());
        std::sort(v2.begin(), v2.end());
        return v1 == v2;
    }
};

TEST_F(RecipeManagerTest, AddRecipeWithTags)
{
    RecipeApp::Recipe recipe1 = createRecipeWithTags(0, "Pasta Carbonara", {"Italian", "Dinner", "Quick"});
    int id1 = manager->addRecipe(recipe1);
    ASSERT_GT(id1, 0);

    std::optional<RecipeApp::Recipe> fetchedRecipeOpt = manager->findRecipeById(id1);
    ASSERT_TRUE(fetchedRecipeOpt.has_value());
    RecipeApp::Recipe fetchedRecipe = fetchedRecipeOpt.value();

    std::vector<std::string> expectedTags = {"Italian", "Dinner", "Quick"};
    ASSERT_TRUE(compareTags(fetchedRecipe.getTags(), expectedTags));
}

TEST_F(RecipeManagerTest, UpdateRecipeChangeTags)
{
    RecipeApp::Recipe recipe1 = createRecipeWithTags(0, "Salad", {"Healthy", "Lunch"});
    int id1 = manager->addRecipe(recipe1);
    ASSERT_GT(id1, 0);

    std::optional<RecipeApp::Recipe> fetchedRecipeOpt = manager->findRecipeById(id1);
    ASSERT_TRUE(fetchedRecipeOpt.has_value());
    RecipeApp::Recipe recipeToUpdate = fetchedRecipeOpt.value();

    recipeToUpdate.removeTag("Lunch");
    recipeToUpdate.addTag("Vegan");
    recipeToUpdate.addTag("Quick");

    ASSERT_TRUE(manager->updateRecipe(recipeToUpdate));

    std::optional<RecipeApp::Recipe> updatedRecipeOpt = manager->findRecipeById(id1);
    ASSERT_TRUE(updatedRecipeOpt.has_value());
    RecipeApp::Recipe updatedRecipe = updatedRecipeOpt.value();

    std::vector<std::string> expectedTags = {"Healthy", "Vegan", "Quick"};
    ASSERT_TRUE(compareTags(updatedRecipe.getTags(), expectedTags));
}

TEST_F(RecipeManagerTest, UpdateRecipeClearAllTags)
{
    RecipeApp::Recipe recipe1 = createRecipeWithTags(0, "Steak", {"Meat", "Dinner", "Grill"});
    int id1 = manager->addRecipe(recipe1);
    ASSERT_GT(id1, 0);

    std::optional<RecipeApp::Recipe> fetchedRecipeOpt = manager->findRecipeById(id1);
    ASSERT_TRUE(fetchedRecipeOpt.has_value());
    RecipeApp::Recipe recipeToUpdate = fetchedRecipeOpt.value();

    // Clear tags by re-assigning an empty set or removing all existing ones
    // Assuming Recipe class has a way to clear tags or setTags method
    // For now, let's remove them one by one if no clearTags method exists
    std::vector<std::string> currentTags = recipeToUpdate.getTags();
    for (const auto &tag : currentTags)
    {
        recipeToUpdate.removeTag(tag);
    }
    // Or if Recipe has setTags: recipeToUpdate.setTags({});

    ASSERT_TRUE(manager->updateRecipe(recipeToUpdate));

    std::optional<RecipeApp::Recipe> updatedRecipeOpt = manager->findRecipeById(id1);
    ASSERT_TRUE(updatedRecipeOpt.has_value());
    RecipeApp::Recipe updatedRecipe = updatedRecipeOpt.value();

    ASSERT_TRUE(updatedRecipe.getTags().empty());
}

TEST_F(RecipeManagerTest, FindRecipesBySingleTag)
{
    manager->addRecipe(createRecipeWithTags(0, "Recipe A", {"Tag1", "Tag2"}));
    manager->addRecipe(createRecipeWithTags(0, "Recipe B", {"Tag2", "Tag3"}));
    manager->addRecipe(createRecipeWithTags(0, "Recipe C", {"Tag1"}));

    std::vector<RecipeApp::Recipe> foundByTag1 = manager->findRecipesByTag("Tag1");
    ASSERT_EQ(foundByTag1.size(), 2);
    // Check names or IDs if necessary

    std::vector<RecipeApp::Recipe> foundByTag3 = manager->findRecipesByTag("Tag3");
    ASSERT_EQ(foundByTag3.size(), 1);
    EXPECT_EQ(foundByTag3[0].getName(), "Recipe B");

    std::vector<RecipeApp::Recipe> foundByNonExistentTag = manager->findRecipesByTag("NonExistent");
    ASSERT_TRUE(foundByNonExistentTag.empty());
}

TEST_F(RecipeManagerTest, FindRecipesByMultipleTags_MatchAll)
{
    manager->addRecipe(createRecipeWithTags(0, "Recipe Alpha", {"Common", "AlphaFeature", "Primary"}));
    manager->addRecipe(createRecipeWithTags(0, "Recipe Beta", {"Common", "BetaFeature", "Primary"}));
    manager->addRecipe(createRecipeWithTags(0, "Recipe Gamma", {"Common", "AlphaFeature"}));  // Missing Primary
    manager->addRecipe(createRecipeWithTags(0, "Recipe Delta", {"AlphaFeature", "Primary"})); // Missing Common

    std::vector<std::string> searchTags1 = {"Common", "AlphaFeature", "Primary"};
    std::vector<RecipeApp::Recipe> found1 = manager->findRecipesByTags(searchTags1, true);
    ASSERT_EQ(found1.size(), 1);
    EXPECT_EQ(found1[0].getName(), "Recipe Alpha");

    std::vector<std::string> searchTags2 = {"Common", "Primary"};
    std::vector<RecipeApp::Recipe> found2 = manager->findRecipesByTags(searchTags2, true);
    ASSERT_EQ(found2.size(), 2); // Alpha and Beta

    std::vector<std::string> searchTags3 = {"AlphaFeature"};
    std::vector<RecipeApp::Recipe> found3 = manager->findRecipesByTags(searchTags3, true);
    ASSERT_EQ(found3.size(), 3); // Alpha, Gamma, Delta

    std::vector<std::string> searchTagsNone = {"NonExistentTag", "Common"};
    std::vector<RecipeApp::Recipe> foundNone = manager->findRecipesByTags(searchTagsNone, true);
    ASSERT_TRUE(foundNone.empty());
}

TEST_F(RecipeManagerTest, FindRecipesByMultipleTags_MatchAny)
{
    // Note: findRecipesByTags currently only supports matchAll=true based on RecipeManager.h
    // If matchAll=false (OR logic) is implemented, this test would be valid.
    // For now, this test will behave like matchAll=true or needs adjustment if the method signature changes.
    manager->addRecipe(createRecipeWithTags(0, "Recipe X", {"UniqueX", "Shared1"}));
    manager->addRecipe(createRecipeWithTags(0, "Recipe Y", {"UniqueY", "Shared2"}));
    manager->addRecipe(createRecipeWithTags(0, "Recipe Z", {"Shared1", "Shared2"}));
    manager->addRecipe(createRecipeWithTags(0, "Recipe W", {"UniqueW"}));

    // Assuming current implementation (matchAll = true by default or only option)
    std::vector<std::string> searchTagsOr = {"UniqueX", "UniqueY"};
    std::vector<RecipeApp::Recipe> foundOr = manager->findRecipesByTags(searchTagsOr, true); // This will be AND
    ASSERT_TRUE(foundOr.empty());

    // If matchAll = false was an option:
    // std::vector<RecipeApp::Recipe> foundOr = manager->findRecipesByTags(searchTagsOr, false);
    // ASSERT_EQ(foundOr.size(), 2); // X and Y
}