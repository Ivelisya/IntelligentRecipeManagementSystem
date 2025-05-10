#include <algorithm>  // For std::sort, std::equal
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "domain/recipe/Recipe.h"
#include "gtest/gtest.h"
#include "logic/recipe/RecipeManager.h"
#include "persistence/JsonRecipeRepository.h"  // For concrete repository in tests
#include "common/exceptions/ValidationException.h" // For testing exception throws
#include "gmock/gmock.h" // For GMock framework
#include "domain/recipe/RecipeRepository.h" // Base class for mock

// Mock RecipeRepository
class MockRecipeRepository : public RecipeApp::Domain::Recipe::RecipeRepository {
public:
    MOCK_CONST_METHOD1(findById, std::optional<RecipeApp::Recipe>(int recipeId));
    MOCK_CONST_METHOD2(findByName, std::vector<RecipeApp::Recipe>(const std::string& name, bool partialMatch));
    MOCK_CONST_METHOD0(findAll, std::vector<RecipeApp::Recipe>());
    MOCK_METHOD1(save, int(const RecipeApp::Recipe& recipe));
    MOCK_METHOD1(remove, bool(int recipeId));
    MOCK_CONST_METHOD1(findManyByIds, std::vector<RecipeApp::Recipe>(const std::vector<int>& ids));
    MOCK_CONST_METHOD1(findByTag, std::vector<RecipeApp::Recipe>(const std::string& tagName));
    MOCK_CONST_METHOD2(findByIngredients, std::vector<RecipeApp::Recipe>(const std::vector<std::string>& ingredientNames, bool matchAll));
    MOCK_CONST_METHOD2(findByTags, std::vector<RecipeApp::Recipe>(const std::vector<std::string>& tagNames, bool matchAll));
    MOCK_METHOD1(setNextId, void(int nextId));
};

// Helper to create a recipe with specific tags
RecipeApp::Recipe createRecipeWithTags(
    int id, const std::string &name, const std::vector<std::string> &tags_param,
    const std::string &cuisine_tag_param =
        "TestCuisine")  // Renamed params for clarity
{
    std::vector<std::string> all_tags = tags_param;
    if (!cuisine_tag_param
             .empty()) {  // Add cuisine_tag_param if it's not empty
        // Avoid adding if it's already in tags_param to prevent duplicates from
        // this specific logic
        bool cuisine_tag_exists = false;
        for (const auto &t : tags_param) {
            if (t == cuisine_tag_param) {
                cuisine_tag_exists = true;
                break;
            }
        }
        if (!cuisine_tag_exists) {
            all_tags.push_back(cuisine_tag_param);
        }
    }

    return RecipeApp::Recipe::builder(id, name)
        .withIngredients({RecipeApp::Ingredient{
            "ingredient", "1g"}})  // Use Ingredient struct
        .withSteps({"step 1"})
        .withCookingTime(10)
        .withDifficulty(RecipeApp::Difficulty::Easy)
        .withTags(all_tags)  // Pass the combined list of tags
        .build();
}

class RecipeManagerTest : public ::testing::Test {
   protected:
    // std::filesystem::path tempTestDir; // No longer needed for mock
    // const std::string testDbFileName = "recipes_manager_test.json"; // No longer needed
    std::shared_ptr<MockRecipeRepository> mockRepo; // Changed to use MockRecipeRepository
    std::unique_ptr<RecipeApp::RecipeManager> manager;

    void SetUp() override {
        // Create a new mock repository for each test
        mockRepo = std::make_shared<MockRecipeRepository>();
        
        // Expect findAll to be called during RecipeManager construction (by loadRecipes).
        // This sets up a clean initial state for the manager's internal caches/indexes.
        EXPECT_CALL(*mockRepo, findAll())
            .WillRepeatedly(testing::Return(std::vector<RecipeApp::Recipe>{}));
            
        // Initialize RecipeManager with the mock repository
        manager = std::make_unique<RecipeApp::RecipeManager>(*mockRepo);
    }

    void TearDown() override {
        // mockRepo and manager will be destroyed automatically by smart pointers
        // No file system cleanup needed for mock-based tests
    }

    // Helper to compare two vectors of strings, ignoring order
    bool compareTags(std::vector<std::string> v1, std::vector<std::string> v2) {
        std::sort(v1.begin(), v1.end());
        std::sort(v2.begin(), v2.end());
        return v1 == v2;
    }
};

TEST_F(RecipeManagerTest, AddRecipeWithTags) {
    RecipeApp::Recipe recipe_to_add = createRecipeWithTags(
        0, "Pasta Carbonara", {"Italian", "Dinner", "Quick"});
    int expected_new_id = 1;

    RecipeApp::Recipe recipe_as_if_saved_with_id = RecipeApp::Recipe::builder(expected_new_id, recipe_to_add.getName())
                                       .withIngredients(recipe_to_add.getIngredients())
                                       .withSteps(recipe_to_add.getSteps())
                                       .withCookingTime(recipe_to_add.getCookingTime())
                                       .withDifficulty(recipe_to_add.getDifficulty())
                                       .withTags(recipe_to_add.getTags())
                                       .build();

    // Based on new RecipeManager logic:
    // 1. No direct call to mockRepo->findByName for addRecipe's name conflict check. It uses internal index.
    // 2. mockRepo->save IS called.
    // 3. mockRepo->findById IS called after save for indexing.
    // 4. No call to mockRepo->setNextId.

    {
        testing::InSequence seq;
        EXPECT_CALL(*mockRepo, save(testing::An<const RecipeApp::Recipe&>()))
            .WillOnce(testing::Return(expected_new_id));

        EXPECT_CALL(*mockRepo, findById(expected_new_id)) // For manager's internal indexing
            .WillOnce(testing::Return(std::make_optional(recipe_as_if_saved_with_id)));
    }
    
    int actual_new_id = manager->addRecipe(recipe_to_add);
    ASSERT_EQ(actual_new_id, expected_new_id);

    // Verification call by the test
    EXPECT_CALL(*mockRepo, findById(expected_new_id))
        .WillOnce(testing::Return(std::make_optional(recipe_as_if_saved_with_id)));
        
    std::optional<RecipeApp::Recipe> fetchedRecipeOpt = manager->findRecipeById(expected_new_id);
    ASSERT_TRUE(fetchedRecipeOpt.has_value());
    RecipeApp::Recipe fetchedRecipe = fetchedRecipeOpt.value();

    std::vector<std::string> expectedTags = {"Italian", "Dinner", "Quick", "TestCuisine"};
    ASSERT_TRUE(compareTags(fetchedRecipe.getTags(), expectedTags));
    ASSERT_EQ(fetchedRecipe.getRecipeId(), expected_new_id);
    ASSERT_EQ(fetchedRecipe.getName(), "Pasta Carbonara");
}

TEST_F(RecipeManagerTest, AddRecipe_NameConflict) {
    RecipeApp::Recipe recipe1_to_add = createRecipeWithTags(0, "Unique Name", {"TagA"});
    int expected_id1 = 1;
    RecipeApp::Recipe recipe1_as_if_saved = RecipeApp::Recipe::builder(expected_id1, recipe1_to_add.getName())
                                       .withIngredients(recipe1_to_add.getIngredients())
                                       .withSteps(recipe1_to_add.getSteps())
                                       .withCookingTime(recipe1_to_add.getCookingTime())
                                       .withDifficulty(recipe1_to_add.getDifficulty())
                                       .withTags(recipe1_to_add.getTags())
                                       .build();
    {
        testing::InSequence seq;
        // Expectations for the first addRecipe call (successful)
        // NO findByName call to repo, name check is internal via m_nameIndex (empty at start)
        EXPECT_CALL(*mockRepo, save(testing::An<const RecipeApp::Recipe&>()))
            .WillOnce(testing::Return(expected_id1));
        EXPECT_CALL(*mockRepo, findById(expected_id1)) // For internal indexing
            .WillOnce(testing::Return(std::make_optional(recipe1_as_if_saved)));
        // NO setNextId call to repo
    }
    int id1 = manager->addRecipe(recipe1_to_add);
    ASSERT_EQ(id1, expected_id1);

    // For the second addRecipe call (name conflict):
    // RecipeManager will check its m_nameIndex, find recipe1_to_add, and throw.
    // NO calls to mockRepo are expected for this conflicting add.
    RecipeApp::Recipe recipe2_conflict = createRecipeWithTags(0, "Unique Name", {"TagB"});
    
    EXPECT_CALL(*mockRepo, findByName(testing::_, testing::_)).Times(0); // Should not be called
    EXPECT_CALL(*mockRepo, save(testing::_)).Times(0);                   // Should not be called
    EXPECT_CALL(*mockRepo, findById(testing::_)).Times(0);              // Should not be called
    EXPECT_CALL(*mockRepo, setNextId(testing::_)).Times(0);             // Should not be called

    ASSERT_THROW(manager->addRecipe(recipe2_conflict), RecipeApp::Common::Exceptions::ValidationException);
}

TEST_F(RecipeManagerTest, UpdateRecipe_NotFound) {
    RecipeApp::Recipe recipe_non_existent = createRecipeWithTags(999, "Non Existent", {"TagX"});
    
    // Expect findById to be called by updateRecipe
    // and return nullopt because the recipe does not exist.
    EXPECT_CALL(*mockRepo, findById(999))
        .WillOnce(testing::Return(std::nullopt)); // updateRecipe calls findById once.
        
    // No other repository methods (like save or findByName) should be called if the recipe is not found for update.
    EXPECT_CALL(*mockRepo, save(testing::_)).Times(0);
    // findByName might be called by updateRecipe if it proceeds further, but it shouldn't if findById returns nullopt.
    // To be safe, we can specify Times(0) if we are sure.
    // However, RecipeManager::updateRecipe first calls findById. If that fails, it returns false.
    // If it succeeds, it then might call findByName for name conflict check.
    // So, for a "Not Found" case, findByName should not be called.
    EXPECT_CALL(*mockRepo, findByName(testing::_, testing::_)).Times(0);


    // The explicit ASSERT_FALSE(manager->findRecipeById(999).has_value()); is fine, it just reuses the findById mock.
    // Let's test the updateRecipe call directly.
    ASSERT_FALSE(manager->updateRecipe(recipe_non_existent));
}

TEST_F(RecipeManagerTest, UpdateRecipe_NameConflict) {
    // --- Setup: Define recipe data ---
    RecipeApp::Recipe recipe1_orig_data = createRecipeWithTags(0, "Original Name 1", {"Tag1"});
    int id1 = 1;
    RecipeApp::Recipe recipe1_saved_state = RecipeApp::Recipe::builder(id1, recipe1_orig_data.getName())
                                        .withTags(recipe1_orig_data.getTags()).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).withIngredients({{"i","q"}}).withSteps({"s"}).build();

    RecipeApp::Recipe recipe2_orig_data = createRecipeWithTags(0, "Original Name 2", {"Tag2"});
    int id2 = 2;
    RecipeApp::Recipe recipe2_saved_state = RecipeApp::Recipe::builder(id2, recipe2_orig_data.getName())
                                        .withTags(recipe2_orig_data.getTags()).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).withIngredients({{"i","q"}}).withSteps({"s"}).build();

    // --- Mocking the first add operation (recipe1) ---
    {
        testing::InSequence seq_add1;
        // NO findByName call to repo for addRecipe
        EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){ return r.getName() == recipe1_orig_data.getName(); })))
            .WillOnce(testing::Return(id1));
        EXPECT_CALL(*mockRepo, findById(id1)) // For addRecipe's internal indexing
            .WillOnce(testing::Return(std::make_optional(recipe1_saved_state)));
        // NO setNextId
    }
    manager->addRecipe(recipe1_orig_data);

    // --- Mocking the second add operation (recipe2) ---
    {
        testing::InSequence seq_add2;
        // NO findByName call to repo for addRecipe
        EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){ return r.getName() == recipe2_orig_data.getName(); })))
            .WillOnce(testing::Return(id2));
        EXPECT_CALL(*mockRepo, findById(id2)) // For addRecipe's internal indexing
            .WillOnce(testing::Return(std::make_optional(recipe2_saved_state)));
        // NO setNextId
    }
    manager->addRecipe(recipe2_orig_data);

    // --- Test: Try to update recipe2's name to recipe1's name ---
    RecipeApp::Recipe recipe2_to_update = recipe2_saved_state;
    recipe2_to_update.setName(recipe1_saved_state.getName());

    // Expectations for the updateRecipe call (conflicting)
    // 1. updateRecipe calls findById to get the original recipe.
    EXPECT_CALL(*mockRepo, findById(id2))
        .WillOnce(testing::Return(std::make_optional(recipe2_saved_state)));
    // 2. Name conflict is detected internally by RecipeManager using m_nameIndex.
    //    NO call to mockRepo->findByName.
    //    NO call to mockRepo->save for update.
    EXPECT_CALL(*mockRepo, findByName(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*mockRepo, save(testing::_)).Times(0);
    
    ASSERT_FALSE(manager->updateRecipe(recipe2_to_update));

    // --- Verification: Ensure recipe2 in "storage" (mock) is unchanged ---
    EXPECT_CALL(*mockRepo, findById(id2))
        .WillOnce(testing::Return(std::make_optional(recipe2_saved_state)));
        
    std::optional<RecipeApp::Recipe> recipe2_after_failed_update_opt = manager->findRecipeById(id2);
    ASSERT_TRUE(recipe2_after_failed_update_opt.has_value());
    ASSERT_EQ(recipe2_after_failed_update_opt.value().getName(), "Original Name 2");
}

TEST_F(RecipeManagerTest, DeleteRecipe_NotFound) {
    int non_existent_id = 999;
    
    // Expect findById to be called by deleteRecipe and return nullopt
    EXPECT_CALL(*mockRepo, findById(non_existent_id))
        .WillOnce(testing::Return(std::nullopt));
        
    // remove should NOT be called if the recipe is not found.
    EXPECT_CALL(*mockRepo, remove(non_existent_id)).Times(0);

    ASSERT_FALSE(manager->deleteRecipe(non_existent_id));
}

TEST_F(RecipeManagerTest, UpdateRecipeChangeTags) {
    int recipe_id = 1;
    RecipeApp::Recipe recipe_initial = createRecipeWithTags(recipe_id, "Salad", {"Healthy", "Lunch"});
    
    RecipeApp::Recipe recipeToUpdate = recipe_initial;
    recipeToUpdate.removeTag("Lunch");
    recipeToUpdate.addTag("Vegan");
    recipeToUpdate.addTag("Quick");
    // Expected tags after update: {"Healthy", "TestCuisine", "Vegan", "Quick"} - order doesn't matter due to compareTags

    RecipeApp::Recipe recipe_final_state = recipeToUpdate;

    {
        testing::InSequence seq;

        // 1. updateRecipe calls findById to get the original recipe
        EXPECT_CALL(*mockRepo, findById(recipe_id))
            .WillOnce(testing::Return(std::make_optional(recipe_initial)));

        // 2. Name is not changed, so findByName is not called by RecipeManager.
        
        // 3. save (to persist the updated recipe)
        EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){
            return r.getRecipeId() == recipe_id &&
                   r.getName() == "Salad" &&
                   compareTags(r.getTags(), {"Healthy", "TestCuisine", "Vegan", "Quick"});
        })))
            .WillOnce(testing::Return(recipe_id));

        // 4. RecipeManager::updateRecipe does NOT call findById again after save for indexing.
        //    It uses the passed updated_recipe_param and the existingRecipe fetched earlier.
    }

    ASSERT_TRUE(manager->updateRecipe(recipeToUpdate));

    // --- Verification: Fetch again and check tags ---
    EXPECT_CALL(*mockRepo, findById(recipe_id))
        .WillOnce(testing::Return(std::make_optional(recipe_final_state)));

    std::optional<RecipeApp::Recipe> updatedRecipeOpt = manager->findRecipeById(recipe_id);
    ASSERT_TRUE(updatedRecipeOpt.has_value());
    
    std::vector<std::string> expectedTags = {"Healthy", "TestCuisine", "Vegan", "Quick"};
    ASSERT_TRUE(compareTags(updatedRecipeOpt.value().getTags(), expectedTags));
}

TEST_F(RecipeManagerTest, UpdateRecipeClearAllTags) {
    int recipe_id = 1;
    RecipeApp::Recipe recipe_initial = createRecipeWithTags(recipe_id, "Steak", {"Meat", "Dinner", "Grill", "TestCuisine"});
    
    RecipeApp::Recipe recipeToUpdate = recipe_initial;
    recipeToUpdate.setTags({}); // Clear all tags

    RecipeApp::Recipe recipe_final_state = recipeToUpdate; // Recipe with empty tags

    {
        testing::InSequence seq;

        // 1. updateRecipe calls findById to get the original recipe
        EXPECT_CALL(*mockRepo, findById(recipe_id))
            .WillOnce(testing::Return(std::make_optional(recipe_initial)));

        // 2. Name is not changed, so findByName is not called.
        // EXPECT_CALL(*mockRepo, findByName(recipeToUpdate.getName(), false)).Times(0);

        // 3. save (to persist the updated recipe with empty tags)
        EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){
            return r.getRecipeId() == recipe_id &&
                   r.getName() == "Steak" &&
                   r.getTags().empty();
        })))
            .WillOnce(testing::Return(recipe_id));

        // 4. RecipeManager::updateRecipe does NOT call findById again after save for indexing.
    }

    ASSERT_TRUE(manager->updateRecipe(recipeToUpdate));

    // --- Verification: Fetch again and check tags ---
    EXPECT_CALL(*mockRepo, findById(recipe_id))
        .WillOnce(testing::Return(std::make_optional(recipe_final_state)));
        
    std::optional<RecipeApp::Recipe> updatedRecipeOpt = manager->findRecipeById(recipe_id);
    ASSERT_TRUE(updatedRecipeOpt.has_value());
    ASSERT_TRUE(updatedRecipeOpt.value().getTags().empty());
}

TEST_F(RecipeManagerTest, FindRecipesBySingleTag) {
    // Data setup
    RecipeApp::Recipe recipeA_data = createRecipeWithTags(0, "Recipe A", {"Tag1", "Tag2", "TestCuisine"});
    RecipeApp::Recipe recipeB_data = createRecipeWithTags(0, "Recipe B", {"Tag2", "Tag3", "TestCuisine"});
    RecipeApp::Recipe recipeC_data = createRecipeWithTags(0, "Recipe C", {"Tag1", "TestCuisine"});
    
    RecipeApp::Recipe recipeA_saved = RecipeApp::Recipe::builder(1, "Recipe A").withTags({"Tag1", "Tag2", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();
    RecipeApp::Recipe recipeB_saved = RecipeApp::Recipe::builder(2, "Recipe B").withTags({"Tag2", "Tag3", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();
    RecipeApp::Recipe recipeC_saved = RecipeApp::Recipe::builder(3, "Recipe C").withTags({"Tag1", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();

    // Mock addRecipe calls to populate manager's internal index
    EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){ return r.getName() == "Recipe A"; })))
        .WillOnce(testing::Return(1));
    EXPECT_CALL(*mockRepo, findById(1)).WillOnce(testing::Return(std::make_optional(recipeA_saved)));
    manager->addRecipe(recipeA_data);

    EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){ return r.getName() == "Recipe B"; })))
        .WillOnce(testing::Return(2));
    EXPECT_CALL(*mockRepo, findById(2)).WillOnce(testing::Return(std::make_optional(recipeB_saved)));
    manager->addRecipe(recipeB_data);

    EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){ return r.getName() == "Recipe C"; })))
        .WillOnce(testing::Return(3));
    EXPECT_CALL(*mockRepo, findById(3)).WillOnce(testing::Return(std::make_optional(recipeC_saved)));
    manager->addRecipe(recipeC_data);

    // Test findRecipesByTag("Tag1")
    // Expect findManyByIds to be called with IDs of recipes A and C (e.g., {1, 3} or {3, 1})
    std::vector<int> ids_for_tag1 = {1, 3};
    std::vector<RecipeApp::Recipe> result_for_tag1 = {recipeA_saved, recipeC_saved};
    EXPECT_CALL(*mockRepo, findManyByIds(testing::UnorderedElementsAreArray(ids_for_tag1)))
        .WillOnce(testing::Return(result_for_tag1));
    std::vector<RecipeApp::Recipe> foundByTag1 = manager->findRecipesByTag("Tag1");
    ASSERT_EQ(foundByTag1.size(), 2);

    // Test findRecipesByTag("Tag3")
    std::vector<int> ids_for_tag3 = {2};
    std::vector<RecipeApp::Recipe> result_for_tag3 = {recipeB_saved};
    EXPECT_CALL(*mockRepo, findManyByIds(testing::ElementsAreArray(ids_for_tag3)))
        .WillOnce(testing::Return(result_for_tag3));
    std::vector<RecipeApp::Recipe> foundByTag3 = manager->findRecipesByTag("Tag3");
    ASSERT_EQ(foundByTag3.size(), 1);
    EXPECT_EQ(foundByTag3[0].getName(), "Recipe B");

    // Test findRecipesByTag("NonExistent")
    // Manager's index won't find it, so findManyByIds won't be called with non-empty IDs.
    // It might be called with an empty vector, or not at all if manager returns empty directly.
    // If manager's index lookup for "NonExistent" is empty, it returns {} without calling repo.
    // So, no EXPECT_CALL for findManyByIds is needed here if the tag isn't in the index.
    std::vector<RecipeApp::Recipe> foundByNonExistentTag = manager->findRecipesByTag("NonExistent");
    ASSERT_TRUE(foundByNonExistentTag.empty());
}

TEST_F(RecipeManagerTest, FindRecipesByMultipleTags_MatchAll) {
    // Data setup
    RecipeApp::Recipe r_alpha_data = createRecipeWithTags(0, "Recipe Alpha", {"Common", "AlphaFeature", "Primary", "TestCuisine"});
    RecipeApp::Recipe r_beta_data  = createRecipeWithTags(0, "Recipe Beta",  {"Common", "BetaFeature",  "Primary", "TestCuisine"});
    RecipeApp::Recipe r_gamma_data = createRecipeWithTags(0, "Recipe Gamma", {"Common", "AlphaFeature", "TestCuisine"});
    RecipeApp::Recipe r_delta_data = createRecipeWithTags(0, "Recipe Delta", {"AlphaFeature", "Primary", "TestCuisine"});

    RecipeApp::Recipe r_alpha_saved = RecipeApp::Recipe::builder(1, "Recipe Alpha").withTags({"Common", "AlphaFeature", "Primary", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();
    RecipeApp::Recipe r_beta_saved  = RecipeApp::Recipe::builder(2, "Recipe Beta").withTags({"Common", "BetaFeature",  "Primary", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();
    RecipeApp::Recipe r_gamma_saved = RecipeApp::Recipe::builder(3, "Recipe Gamma").withTags({"Common", "AlphaFeature", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();
    RecipeApp::Recipe r_delta_saved = RecipeApp::Recipe::builder(4, "Recipe Delta").withTags({"AlphaFeature", "Primary", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();

    // Mock addRecipe calls
    EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){ return r.getName() == "Recipe Alpha"; }))).WillOnce(testing::Return(1));
    EXPECT_CALL(*mockRepo, findById(1)).WillOnce(testing::Return(std::make_optional(r_alpha_saved)));
    manager->addRecipe(r_alpha_data);

    EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){ return r.getName() == "Recipe Beta"; }))).WillOnce(testing::Return(2));
    EXPECT_CALL(*mockRepo, findById(2)).WillOnce(testing::Return(std::make_optional(r_beta_saved)));
    manager->addRecipe(r_beta_data);

    EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){ return r.getName() == "Recipe Gamma"; }))).WillOnce(testing::Return(3));
    EXPECT_CALL(*mockRepo, findById(3)).WillOnce(testing::Return(std::make_optional(r_gamma_saved)));
    manager->addRecipe(r_gamma_data);

    EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){ return r.getName() == "Recipe Delta"; }))).WillOnce(testing::Return(4));
    EXPECT_CALL(*mockRepo, findById(4)).WillOnce(testing::Return(std::make_optional(r_delta_saved)));
    manager->addRecipe(r_delta_data);

    // Case 1: {"Common", "AlphaFeature", "Primary"} -> Recipe Alpha
    std::vector<std::string> searchTags1 = {"Common", "AlphaFeature", "Primary"};
    std::vector<int> ids_case1 = {1}; // Only Recipe Alpha has all three
    std::vector<RecipeApp::Recipe> result_case1 = {r_alpha_saved};
    EXPECT_CALL(*mockRepo, findManyByIds(testing::UnorderedElementsAreArray(ids_case1)))
        .WillOnce(testing::Return(result_case1));
    std::vector<RecipeApp::Recipe> found1 = manager->findRecipesByTags(searchTags1, true);
    ASSERT_EQ(found1.size(), 1);
    if (!found1.empty()) EXPECT_EQ(found1[0].getName(), "Recipe Alpha");

    // Case 2: {"Common", "Primary"} -> Recipe Alpha, Recipe Beta
    std::vector<std::string> searchTags2 = {"Common", "Primary"};
    std::vector<int> ids_case2 = {1, 2};
    std::vector<RecipeApp::Recipe> result_case2 = {r_alpha_saved, r_beta_saved};
    EXPECT_CALL(*mockRepo, findManyByIds(testing::UnorderedElementsAreArray(ids_case2)))
        .WillOnce(testing::Return(result_case2));
    std::vector<RecipeApp::Recipe> found2 = manager->findRecipesByTags(searchTags2, true);
    ASSERT_EQ(found2.size(), 2);

    // Case 3: {"AlphaFeature"} -> Recipe Alpha, Recipe Gamma, Recipe Delta
    std::vector<std::string> searchTags3 = {"AlphaFeature"};
    std::vector<int> ids_case3 = {1, 3, 4};
    std::vector<RecipeApp::Recipe> result_case3 = {r_alpha_saved, r_gamma_saved, r_delta_saved};
    EXPECT_CALL(*mockRepo, findManyByIds(testing::UnorderedElementsAreArray(ids_case3)))
        .WillOnce(testing::Return(result_case3));
    std::vector<RecipeApp::Recipe> found3 = manager->findRecipesByTags(searchTags3, true);
    ASSERT_EQ(found3.size(), 3);

    // Case 4: {"NonExistentTag", "Common"} -> Empty
    // Manager will find no IDs for "NonExistentTag", so intersection will be empty.
    // findManyByIds should not be called if the derived ID list is empty.
    std::vector<std::string> searchTagsNone = {"NonExistentTag", "Common"};
    std::vector<RecipeApp::Recipe> foundNone = manager->findRecipesByTags(searchTagsNone, true);
    ASSERT_TRUE(foundNone.empty());
}

TEST_F(RecipeManagerTest, FindRecipesByMultipleTags_MatchAny) {
    // This test's active part currently tests matchAll=true.
    // We'll set up data such that the AND logic yields no results.
    RecipeApp::Recipe r_x_data = createRecipeWithTags(0, "Recipe X", {"UniqueX", "Shared1", "TestCuisine"});
    RecipeApp::Recipe r_y_data = createRecipeWithTags(0, "Recipe Y", {"UniqueY", "Shared2", "TestCuisine"});

    RecipeApp::Recipe r_x_saved = RecipeApp::Recipe::builder(1, "Recipe X").withTags({"UniqueX", "Shared1", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();
    RecipeApp::Recipe r_y_saved = RecipeApp::Recipe::builder(2, "Recipe Y").withTags({"UniqueY", "Shared2", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();

    // Mock addRecipe calls to populate manager's internal index
    EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){ return r.getName() == "Recipe X"; }))).WillOnce(testing::Return(1));
    EXPECT_CALL(*mockRepo, findById(1)).WillOnce(testing::Return(std::make_optional(r_x_saved)));
    manager->addRecipe(r_x_data);

    EXPECT_CALL(*mockRepo, save(testing::Truly([&](const RecipeApp::Recipe& r){ return r.getName() == "Recipe Y"; }))).WillOnce(testing::Return(2));
    EXPECT_CALL(*mockRepo, findById(2)).WillOnce(testing::Return(std::make_optional(r_y_saved)));
    manager->addRecipe(r_y_data);
    
    // Current test logic: manager->findRecipesByTags({"UniqueX", "UniqueY"}, true);
    // RecipeManager will find IDs for "UniqueX" (ID 1) and "UniqueY" (ID 2) from its m_tagIndex.
    // With matchAll=true, it will intersect {1} and {2}, resulting in an empty set of IDs.
    // Thus, findManyByIds should NOT be called by RecipeManager as it will return empty directly.
    std::vector<std::string> searchTagsAndLogic = {"UniqueX", "UniqueY"};
    EXPECT_CALL(*mockRepo, findManyByIds(testing::_)).Times(0);

    std::vector<RecipeApp::Recipe> foundAnd = manager->findRecipesByTags(searchTagsAndLogic, true);
    ASSERT_TRUE(foundAnd.empty());

    // --- The following demonstrates how to test matchAll = false (OR logic) if it were active ---
    // std::vector<std::string> searchTagsOrLogic = {"UniqueX", "UniqueY"};
    // std::vector<int> ids_for_or_logic = {1, 2}; // Union of IDs for "UniqueX" and "UniqueY"
    // std::vector<RecipeApp::Recipe> result_for_or_logic = {r_x_saved, r_y_saved};
    // EXPECT_CALL(*mockRepo, findManyByIds(testing::UnorderedElementsAreArray(ids_for_or_logic)))
    //     .WillOnce(testing::Return(result_for_or_logic));
    // std::vector<RecipeApp::Recipe> foundOr = manager->findRecipesByTags(searchTagsOrLogic, false);
    // ASSERT_EQ(foundOr.size(), 2);
    // // Add more specific checks if needed, e.g., for names or IDs.
    // bool foundX = false, foundY = false;
    // for(const auto& r : found_when_matchall_false) {
    //     if (r.getName() == "Recipe X") foundX = true;
    //     if (r.getName() == "Recipe Y") foundY = true;
    // }
    // ASSERT_TRUE(foundX && foundY);
}
TEST_F(RecipeManagerTest, FindRecipesByMultipleTags_MatchAny_Explicit) {
    RecipeApp::Recipe r1_data = createRecipeWithTags(0, "Recipe MatchAny 1", {"TagA", "TagB", "TestCuisine"});
    RecipeApp::Recipe r2_data = createRecipeWithTags(0, "Recipe MatchAny 2", {"TagB", "TagC", "TestCuisine"});
    RecipeApp::Recipe r3_data = createRecipeWithTags(0, "Recipe MatchAny 3", {"TagC", "TagD", "TestCuisine"});
    RecipeApp::Recipe r4_data = createRecipeWithTags(0, "Recipe MatchAny 4", {"TagE", "TestCuisine"}); // No matching tags

    RecipeApp::Recipe r1_saved = RecipeApp::Recipe::builder(10, "Recipe MatchAny 1").withTags({"TagA", "TagB", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();
    RecipeApp::Recipe r2_saved = RecipeApp::Recipe::builder(11, "Recipe MatchAny 2").withTags({"TagB", "TagC", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();
    RecipeApp::Recipe r3_saved = RecipeApp::Recipe::builder(12, "Recipe MatchAny 3").withTags({"TagC", "TagD", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();
    RecipeApp::Recipe r4_saved = RecipeApp::Recipe::builder(13, "Recipe MatchAny 4").withTags({"TagE", "TestCuisine"}).withIngredients({{"i","q"}}).withSteps({"s"}).withCookingTime(10).withDifficulty(RecipeApp::Difficulty::Easy).build();

    EXPECT_CALL(*mockRepo, save(testing::_)).WillOnce(testing::Return(10)).WillOnce(testing::Return(11)).WillOnce(testing::Return(12)).WillOnce(testing::Return(13));
    EXPECT_CALL(*mockRepo, findById(10)).WillOnce(testing::Return(std::make_optional(r1_saved)));
    EXPECT_CALL(*mockRepo, findById(11)).WillOnce(testing::Return(std::make_optional(r2_saved)));
    EXPECT_CALL(*mockRepo, findById(12)).WillOnce(testing::Return(std::make_optional(r3_saved)));
    EXPECT_CALL(*mockRepo, findById(13)).WillOnce(testing::Return(std::make_optional(r4_saved)));
    manager->addRecipe(r1_data);
    manager->addRecipe(r2_data);
    manager->addRecipe(r3_data);
    manager->addRecipe(r4_data);

    std::vector<std::string> searchTags = {"TagA", "TagD"};
    // Expected: Recipe 1 (has TagA), Recipe 3 (has TagD)
    std::vector<int> expected_ids = {10, 12};
    std::vector<RecipeApp::Recipe> expected_recipes = {r1_saved, r3_saved};

    EXPECT_CALL(*mockRepo, findManyByIds(testing::UnorderedElementsAreArray(expected_ids)))
        .WillOnce(testing::Return(expected_recipes));

    std::vector<RecipeApp::Recipe> foundRecipes = manager->findRecipesByTags(searchTags, false); // matchAll = false
    ASSERT_EQ(foundRecipes.size(), 2);
    // Further checks for specific recipes can be added if order is guaranteed or by checking names/IDs
}

TEST_F(RecipeManagerTest, FindRecipesByIngredients_SingleIngredient) {
    RecipeApp::Recipe recipeWithTomato_data = RecipeApp::Recipe::builder(0, "Tomato Soup")
                                        .withIngredients({{"Tomato", "500g"}, {"Onion", "1"}})
                                        .withSteps({"Cook"}).withCookingTime(30).withDifficulty(RecipeApp::Difficulty::Easy).build();
    RecipeApp::Recipe recipeWithChicken_data = RecipeApp::Recipe::builder(0, "Chicken Curry")
                                         .withIngredients({{"Chicken", "1kg"}, {"Curry Powder", "2tbsp"}})
                                         .withSteps({"Cook"}).withCookingTime(45).withDifficulty(RecipeApp::Difficulty::Medium).build();

    RecipeApp::Recipe recipeWithTomato_saved = RecipeApp::Recipe::builder(20, "Tomato Soup").withIngredients({{"Tomato", "500g"}, {"Onion", "1"}}).withSteps({"Cook"}).withCookingTime(30).withDifficulty(RecipeApp::Difficulty::Easy).build();
    RecipeApp::Recipe recipeWithChicken_saved = RecipeApp::Recipe::builder(21, "Chicken Curry").withIngredients({{"Chicken", "1kg"}, {"Curry Powder", "2tbsp"}}).withSteps({"Cook"}).withCookingTime(45).withDifficulty(RecipeApp::Difficulty::Medium).build();
    
    EXPECT_CALL(*mockRepo, save(testing::_)).WillOnce(testing::Return(20)).WillOnce(testing::Return(21));
    EXPECT_CALL(*mockRepo, findById(20)).WillOnce(testing::Return(std::make_optional(recipeWithTomato_saved)));
    EXPECT_CALL(*mockRepo, findById(21)).WillOnce(testing::Return(std::make_optional(recipeWithChicken_saved)));
    manager->addRecipe(recipeWithTomato_data);
    manager->addRecipe(recipeWithChicken_data);

    std::vector<std::string> searchIngredients = {"Tomato"};
    std::vector<int> expected_ids = {20};
    std::vector<RecipeApp::Recipe> expected_recipes = {recipeWithTomato_saved};

    EXPECT_CALL(*mockRepo, findManyByIds(testing::UnorderedElementsAreArray(expected_ids)))
        .WillOnce(testing::Return(expected_recipes));
    
    std::vector<RecipeApp::Recipe> foundRecipes = manager->findRecipesByIngredients(searchIngredients, true); // matchAll = true (for single ingredient, same as false)
    ASSERT_EQ(foundRecipes.size(), 1);
    if(!foundRecipes.empty()) EXPECT_EQ(foundRecipes[0].getName(), "Tomato Soup");
}

TEST_F(RecipeManagerTest, FindRecipesByIngredients_MultipleIngredients_MatchAll) {
    RecipeApp::Recipe r1_data = RecipeApp::Recipe::builder(0, "Pasta Tomato Basil")
                                .withIngredients({{"Pasta", "200g"}, {"Tomato", "3"}, {"Basil", "1 bunch"}})
                                .withSteps({"Cook"}).build();
    RecipeApp::Recipe r2_data = RecipeApp::Recipe::builder(0, "Tomato Salad")
                                .withIngredients({{"Tomato", "2"}, {"Lettuce", "1 head"}})
                                .withSteps({"Mix"}).build();

    RecipeApp::Recipe r1_saved = RecipeApp::Recipe::builder(30, "Pasta Tomato Basil").withIngredients({{"Pasta", "200g"}, {"Tomato", "3"}, {"Basil", "1 bunch"}}).withSteps({"Cook"}).build();
    RecipeApp::Recipe r2_saved = RecipeApp::Recipe::builder(31, "Tomato Salad").withIngredients({{"Tomato", "2"}, {"Lettuce", "1 head"}}).withSteps({"Mix"}).build();

    EXPECT_CALL(*mockRepo, save(testing::_)).WillOnce(testing::Return(30)).WillOnce(testing::Return(31));
    EXPECT_CALL(*mockRepo, findById(30)).WillOnce(testing::Return(std::make_optional(r1_saved)));
    EXPECT_CALL(*mockRepo, findById(31)).WillOnce(testing::Return(std::make_optional(r2_saved)));
    manager->addRecipe(r1_data);
    manager->addRecipe(r2_data);

    std::vector<std::string> searchIngredients = {"Tomato", "Basil"};
    std::vector<int> expected_ids = {30}; // Only r1 has both Tomato and Basil
    std::vector<RecipeApp::Recipe> expected_recipes = {r1_saved};

    EXPECT_CALL(*mockRepo, findManyByIds(testing::UnorderedElementsAreArray(expected_ids)))
        .WillOnce(testing::Return(expected_recipes));

    std::vector<RecipeApp::Recipe> foundRecipes = manager->findRecipesByIngredients(searchIngredients, true); // matchAll = true
    ASSERT_EQ(foundRecipes.size(), 1);
    if(!foundRecipes.empty()) EXPECT_EQ(foundRecipes[0].getName(), "Pasta Tomato Basil");
}

TEST_F(RecipeManagerTest, FindRecipesByIngredients_MultipleIngredients_MatchAny) {
    RecipeApp::Recipe r1_data = RecipeApp::Recipe::builder(0, "Chicken Stir-fry")
                                .withIngredients({{"Chicken", "500g"}, {"Broccoli", "1 head"}, {"Soy Sauce", "2 tbsp"}})
                                .withSteps({"Cook"}).build();
    RecipeApp::Recipe r2_data = RecipeApp::Recipe::builder(0, "Broccoli Soup")
                                .withIngredients({{"Broccoli", "2 heads"}, {"Cream", "200ml"}})
                                .withSteps({"Cook"}).build();
    RecipeApp::Recipe r3_data = RecipeApp::Recipe::builder(0, "Beef Noodles")
                                .withIngredients({{"Beef", "300g"}, {"Noodles", "200g"}, {"Soy Sauce", "1 tbsp"}})
                                .withSteps({"Cook"}).build();

    RecipeApp::Recipe r1_saved = RecipeApp::Recipe::builder(40, "Chicken Stir-fry").withIngredients({{"Chicken", "500g"}, {"Broccoli", "1 head"}, {"Soy Sauce", "2 tbsp"}}).withSteps({"Cook"}).build();
    RecipeApp::Recipe r2_saved = RecipeApp::Recipe::builder(41, "Broccoli Soup").withIngredients({{"Broccoli", "2 heads"}, {"Cream", "200ml"}}).withSteps({"Cook"}).build();
    RecipeApp::Recipe r3_saved = RecipeApp::Recipe::builder(42, "Beef Noodles").withIngredients({{"Beef", "300g"}, {"Noodles", "200g"}, {"Soy Sauce", "1 tbsp"}}).withSteps({"Cook"}).build();

    EXPECT_CALL(*mockRepo, save(testing::_)).WillOnce(testing::Return(40)).WillOnce(testing::Return(41)).WillOnce(testing::Return(42));
    EXPECT_CALL(*mockRepo, findById(40)).WillOnce(testing::Return(std::make_optional(r1_saved)));
    EXPECT_CALL(*mockRepo, findById(41)).WillOnce(testing::Return(std::make_optional(r2_saved)));
    EXPECT_CALL(*mockRepo, findById(42)).WillOnce(testing::Return(std::make_optional(r3_saved)));
    manager->addRecipe(r1_data);
    manager->addRecipe(r2_data);
    manager->addRecipe(r3_data);

    std::vector<std::string> searchIngredients = {"Chicken", "Cream"};
    // Expected: r1 (has Chicken), r2 (has Cream)
    std::vector<int> expected_ids = {40, 41}; 
    std::vector<RecipeApp::Recipe> expected_recipes = {r1_saved, r2_saved};

    EXPECT_CALL(*mockRepo, findManyByIds(testing::UnorderedElementsAreArray(expected_ids)))
        .WillOnce(testing::Return(expected_recipes));

    std::vector<RecipeApp::Recipe> foundRecipes = manager->findRecipesByIngredients(searchIngredients, false); // matchAll = false
    ASSERT_EQ(foundRecipes.size(), 2);
}