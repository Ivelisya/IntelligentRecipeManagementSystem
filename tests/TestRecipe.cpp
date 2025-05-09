#include "gtest/gtest.h"
#include "domain/recipe/Recipe.h"
#include "json.hpp" // For nlohmann::json for testing serialization

#include <stdexcept> // For std::invalid_argument, std::runtime_error
#include <algorithm> // For std::find in hasTag

// Convenience alias
using json = nlohmann::json;
using namespace RecipeApp;

// Helper function to create a basic valid Recipe object for tests
Recipe createValidRecipe(int id = 1, const std::string &name = "Test Recipe")
{
    return Recipe(id, name, {{"Ingredient1", "1 cup"}}, {"Step 1"}, 30, Difficulty::Medium, "Test Cuisine");
}

TEST(RecipeTest, ConstructorValidations)
{
    // Test valid construction
    ASSERT_NO_THROW(Recipe(1, "Pasta Carbonara", {{"Spaghetti", "200g"}, {"Egg Yolk", "2"}, {"Pancetta", "100g"}, {"Parmesan", "50g"}},
                           {"Cook spaghetti.", "Fry pancetta.", "Mix eggs and cheese.", "Combine all."},
                           25, Difficulty::Medium, "Italian"));

    // Test invalid cooking time (negative)
    ASSERT_THROW(Recipe(2, "Bad Recipe", {}, {}, -10, Difficulty::Easy, "None"), std::invalid_argument);
}

TEST(RecipeTest, SettersValidations)
{
    Recipe recipe = createValidRecipe();

    // Name
    ASSERT_NO_THROW(recipe.setName("New Valid Name"));
    EXPECT_EQ(recipe.getName(), "New Valid Name");
    ASSERT_THROW(recipe.setName(""), std::invalid_argument);

    // Cooking Time
    ASSERT_NO_THROW(recipe.setCookingTime(45));
    EXPECT_EQ(recipe.getCookingTime(), 45);
    ASSERT_THROW(recipe.setCookingTime(-5), std::invalid_argument);
    EXPECT_EQ(recipe.getCookingTime(), 45); // Should remain unchanged

    // Cuisine
    ASSERT_NO_THROW(recipe.setCuisine("New Cuisine"));
    EXPECT_EQ(recipe.getCuisine(), "New Cuisine");
    ASSERT_THROW(recipe.setCuisine(""), std::invalid_argument);

    // Difficulty
    recipe.setDifficulty(Difficulty::Hard);
    EXPECT_EQ(recipe.getDifficulty(), Difficulty::Hard);

    // Ingredients
    std::vector<std::pair<std::string, std::string>> newIngredients = {{"New Ing", "1"}, {"Another Ing", "2kg"}};
    recipe.setIngredients(newIngredients);
    EXPECT_EQ(recipe.getIngredients().size(), 2);
    EXPECT_EQ(recipe.getIngredients()[0].first, "New Ing");

    // Steps
    std::vector<std::string> newSteps = {"New Step 1", "New Step 2"};
    recipe.setSteps(newSteps);
    EXPECT_EQ(recipe.getSteps().size(), 2);
    EXPECT_EQ(recipe.getSteps()[0], "New Step 1");

    // Nutritional Info (optional)
    recipe.setNutritionalInfo("Calories: 500");
    ASSERT_TRUE(recipe.getNutritionalInfo().has_value());
    EXPECT_EQ(recipe.getNutritionalInfo().value(), "Calories: 500");

    // Image URL (optional)
    recipe.setImageUrl("http://example.com/image.jpg");
    ASSERT_TRUE(recipe.getImageUrl().has_value());
    EXPECT_EQ(recipe.getImageUrl().value(), "http://example.com/image.jpg");
}

TEST(RecipeTest, Getters)
{
    Recipe recipe(10, "Getter Test", {{"Water", "1L"}}, {"Boil"}, 5, Difficulty::Easy, "Testing");
    recipe.setNutritionalInfo("Test Nutrition");
    recipe.setImageUrl("test.png");

    EXPECT_EQ(recipe.getRecipeId(), 10);
    EXPECT_EQ(recipe.getName(), "Getter Test");
    EXPECT_EQ(recipe.getIngredients().size(), 1);
    EXPECT_EQ(recipe.getIngredients()[0].first, "Water");
    EXPECT_EQ(recipe.getSteps().size(), 1);
    EXPECT_EQ(recipe.getSteps()[0], "Boil");
    EXPECT_EQ(recipe.getCookingTime(), 5);
    EXPECT_EQ(recipe.getDifficulty(), Difficulty::Easy);
    EXPECT_EQ(recipe.getCuisine(), "Testing");
    ASSERT_TRUE(recipe.getNutritionalInfo().has_value());
    EXPECT_EQ(recipe.getNutritionalInfo().value(), "Test Nutrition");
    ASSERT_TRUE(recipe.getImageUrl().has_value());
    EXPECT_EQ(recipe.getImageUrl().value(), "test.png");
}

TEST(RecipeTest, GetDisplayDetails)
{
    Recipe recipe = createValidRecipe(1, "Detailed Recipe");
    recipe.setNutritionalInfo("Info");
    recipe.setImageUrl("url");

    std::string details = recipe.getDisplayDetails();
    EXPECT_NE(details.find("Recipe ID: 1"), std::string::npos);
    EXPECT_NE(details.find("Name: Detailed Recipe"), std::string::npos);
    EXPECT_NE(details.find("Cuisine: Test Cuisine"), std::string::npos);
    EXPECT_NE(details.find("Difficulty: Medium"), std::string::npos);
    EXPECT_NE(details.find("Cooking Time: 30 minutes"), std::string::npos);
    EXPECT_NE(details.find("Ingredient1: 1 cup"), std::string::npos);
    EXPECT_NE(details.find("1. Step 1"), std::string::npos);
    EXPECT_NE(details.find("Nutritional Info: Info"), std::string::npos);
    EXPECT_NE(details.find("Image URL: url"), std::string::npos);
}

TEST(RecipeTest, JsonSerializationDeserializationFull)
{
    Recipe originalRecipe(101, "Full JSON Test",
                          {{"Flour", "500g"}, {"Sugar", "200g"}},
                          {"Mix dry ingredients", "Add wet ingredients", "Bake"},
                          60, Difficulty::Hard, "Bakery");
    originalRecipe.setNutritionalInfo("High Sugar");
    originalRecipe.setImageUrl("http://baking.com/cake.jpg");
    originalRecipe.addTag("Dessert");
    originalRecipe.addTag("Cake");

    // Serialize
    json recipeJson = originalRecipe; // Uses to_json

    // Check some key fields in JSON
    ASSERT_EQ(recipeJson["id"], 101);
    ASSERT_EQ(recipeJson["name"], "Full JSON Test");
    ASSERT_EQ(recipeJson["cuisine"], "Bakery");
    ASSERT_EQ(recipeJson["cookingTime"], 60);
    ASSERT_EQ(recipeJson["difficulty"], "Hard"); // Serialized as string
    ASSERT_TRUE(recipeJson["ingredients"].is_array());
    ASSERT_EQ(recipeJson["ingredients"].size(), 2);
    ASSERT_EQ(recipeJson["ingredients"][0]["name"], "Flour");
    ASSERT_EQ(recipeJson["ingredients"][0]["quantity"], "500g");
    ASSERT_TRUE(recipeJson["steps"].is_array());
    ASSERT_EQ(recipeJson["steps"].size(), 3);
    ASSERT_EQ(recipeJson["steps"][0], "Mix dry ingredients");
    ASSERT_EQ(recipeJson["nutritionalInfo"], "High Sugar");
    ASSERT_EQ(recipeJson["imageUrl"], "http://baking.com/cake.jpg");
    ASSERT_TRUE(recipeJson["tags"].is_array());
    ASSERT_EQ(recipeJson["tags"].size(), 2);
    EXPECT_EQ(recipeJson["tags"][0], "Dessert"); // Assuming order is preserved or test for presence
    EXPECT_EQ(recipeJson["tags"][1], "Cake");

    // Deserialize
    Recipe deserializedRecipe = recipeJson.get<Recipe>(); // Uses from_json

    EXPECT_EQ(deserializedRecipe.getRecipeId(), originalRecipe.getRecipeId());
    EXPECT_EQ(deserializedRecipe.getName(), originalRecipe.getName());
    EXPECT_EQ(deserializedRecipe.getCuisine(), originalRecipe.getCuisine());
    EXPECT_EQ(deserializedRecipe.getCookingTime(), originalRecipe.getCookingTime());
    EXPECT_EQ(deserializedRecipe.getDifficulty(), originalRecipe.getDifficulty());
    ASSERT_EQ(deserializedRecipe.getIngredients().size(), originalRecipe.getIngredients().size());
    EXPECT_EQ(deserializedRecipe.getIngredients()[0].first, originalRecipe.getIngredients()[0].first);
    ASSERT_EQ(deserializedRecipe.getSteps().size(), originalRecipe.getSteps().size());
    EXPECT_EQ(deserializedRecipe.getSteps()[0], originalRecipe.getSteps()[0]);
    ASSERT_TRUE(deserializedRecipe.getNutritionalInfo().has_value());
    EXPECT_EQ(deserializedRecipe.getNutritionalInfo().value(), originalRecipe.getNutritionalInfo().value());
    ASSERT_TRUE(deserializedRecipe.getImageUrl().has_value());
    EXPECT_EQ(deserializedRecipe.getImageUrl().value(), originalRecipe.getImageUrl().value());
    ASSERT_EQ(deserializedRecipe.getTags().size(), 2);
    EXPECT_TRUE(deserializedRecipe.hasTag("Dessert"));
    EXPECT_TRUE(deserializedRecipe.hasTag("Cake"));
}

TEST(RecipeTest, JsonDeserializationOptionalFieldsMissing)
{
    json j = {
        {"id", 102},
        {"name", "Minimal Recipe"},
        {"cuisine", "Minimalist"},
        {"cookingTime", 5},
        {"difficulty", "Easy"},
        {"ingredients", json::array()},
        {"steps", json::array()}
        // nutritionalInfo, imageUrl, and tags are missing
    };

    Recipe recipe = j.get<Recipe>();

    EXPECT_EQ(recipe.getRecipeId(), 102);
    EXPECT_EQ(recipe.getName(), "Minimal Recipe");
    EXPECT_FALSE(recipe.getNutritionalInfo().has_value());
    EXPECT_FALSE(recipe.getImageUrl().has_value());
    EXPECT_TRUE(recipe.getTags().empty()); // Tags should be empty if missing in JSON
}

TEST(RecipeTest, JsonDeserializationOptionalFieldsNull)
{
    json j = {
        {"id", 103},
        {"name", "Null Optionals Recipe"},
        {"cuisine", "Nullable"},
        {"cookingTime", 15},
        {"difficulty", "Medium"},
        {"ingredients", json::array()}, // Keep it simple for this test
        {"steps", json::array()},
        {"nutritionalInfo", nullptr},
        {"imageUrl", nullptr},
        {"tags", nullptr} // Test tags as null
    };
    j["ingredients"].push_back({{"name", "item"}, {"quantity", "1"}});

    Recipe recipe = j.get<Recipe>();

    EXPECT_EQ(recipe.getRecipeId(), 103);
    EXPECT_FALSE(recipe.getNutritionalInfo().has_value());
    EXPECT_FALSE(recipe.getImageUrl().has_value());
    EXPECT_TRUE(recipe.getTags().empty()); // Tags should be empty if null in JSON
}

TEST(RecipeTest, JsonDeserializationInvalidData)
{
    // Missing ID
    json j_no_id = {
        {"name", "No ID Recipe"},
        {"cuisine", "Invalid"},
        {"cookingTime", 10},
        {"difficulty", "Easy"}};
    ASSERT_THROW(j_no_id.get<Recipe>(), std::runtime_error);

    // Invalid ID (zero)
    json j_zero_id = {
        {"id", 0},
        {"name", "Zero ID Recipe"},
        {"cuisine", "Invalid"},
        {"cookingTime", 10},
        {"difficulty", "Easy"}};
    ASSERT_THROW(j_zero_id.get<Recipe>(), std::runtime_error);

    // Missing Name
    json j_no_name = {
        {"id", 201},
        {"cuisine", "Invalid"},
        {"cookingTime", 10},
        {"difficulty", "Easy"}};
    ASSERT_THROW(j_no_name.get<Recipe>(), std::runtime_error);

    // Empty Name
    json j_empty_name = {
        {"id", 202},
        {"name", ""},
        {"cuisine", "Invalid"},
        {"cookingTime", 10},
        {"difficulty", "Easy"}};
    ASSERT_THROW(j_empty_name.get<Recipe>(), std::runtime_error);

    // Missing cookingTime
    json j_no_time = {
        {"id", 203},
        {"name", "No Time Recipe"},
        {"cuisine", "Invalid"},
        {"difficulty", "Easy"}};
    ASSERT_THROW(j_no_time.get<Recipe>(), std::runtime_error);

    // Negative cookingTime
    json j_neg_time = {
        {"id", 204},
        {"name", "Negative Time Recipe"},
        {"cuisine", "Invalid"},
        {"cookingTime", -10},
        {"difficulty", "Easy"}};
    ASSERT_THROW(j_neg_time.get<Recipe>(), std::runtime_error);

    // Missing difficulty
    json j_no_difficulty = {
        {"id", 205},
        {"name", "No Difficulty Recipe"},
        {"cuisine", "Invalid"},
        {"cookingTime", 10}};
    ASSERT_THROW(j_no_difficulty.get<Recipe>(), std::runtime_error);

    // Invalid difficulty string
    json j_invalid_difficulty = {
        {"id", 206},
        {"name", "Invalid Diff Recipe"},
        {"cuisine", "Valid"},
        {"cookingTime", 10},
        {"difficulty", "SuperHard"} // Not a valid enum string
    };
    Recipe recipe_invalid_diff = j_invalid_difficulty.get<Recipe>();
    EXPECT_EQ(recipe_invalid_diff.getDifficulty(), RecipeApp::Difficulty::Easy); // nlohmann::json defaults to first enum value on parse error
}

TEST(RecipeTest, EqualityOperator)
{
    Recipe recipe1 = createValidRecipe(1, "Recipe One");
    Recipe recipe2 = createValidRecipe(1, "Recipe One Variant"); // Same ID, different name
    Recipe recipe3 = createValidRecipe(2, "Recipe Two");         // Different ID

    EXPECT_TRUE(recipe1 == recipe2); // Equality is based on ID only
    EXPECT_FALSE(recipe1 == recipe3);
    EXPECT_FALSE(recipe2 == recipe3);
}

// --- Tag Management Tests ---
TEST(RecipeTest, AddAndGetTags)
{
    Recipe recipe = createValidRecipe(1, "Tag Test Recipe");
    ASSERT_TRUE(recipe.getTags().empty());

    recipe.addTag("Easy");
    ASSERT_EQ(recipe.getTags().size(), 1);
    EXPECT_EQ(recipe.getTags()[0], "Easy");
    EXPECT_TRUE(recipe.hasTag("Easy"));

    recipe.addTag("Dinner");
    ASSERT_EQ(recipe.getTags().size(), 2);
    EXPECT_TRUE(recipe.hasTag("Easy"));
    EXPECT_TRUE(recipe.hasTag("Dinner"));

    // Adding duplicate tag
    recipe.addTag("Easy"); // Should ideally not add if already present, or Recipe::addTag handles uniqueness
                           // Current Recipe::addTag allows duplicates, so size will be 3.
                           // If addTag is changed to enforce uniqueness, this assertion needs to be 2.
    // Let's assume current behavior allows duplicates for this test, then check for specific unique tag presence.
    // To test uniqueness, we'd check size before and after, or use a set-like structure internally in Recipe.
    // For now, we'll stick to hasTag for verification of presence.
    size_t easy_count = 0;
    for (const auto &t : recipe.getTags())
        if (t == "Easy")
            easy_count++;
    EXPECT_GE(easy_count, 1);              // At least one "Easy" tag
    EXPECT_EQ(recipe.getTags().size(), 3); // If duplicates are allowed by addTag

    // Adding empty tag
    recipe.addTag("");
    // Assuming Recipe::addTag ignores empty strings based on its implementation
    bool foundEmpty = false;
    for (const auto &tag : recipe.getTags())
    {
        if (tag.empty())
            foundEmpty = true;
    }
    EXPECT_FALSE(foundEmpty);
    EXPECT_EQ(recipe.getTags().size(), 3); // Size should not change if empty tags are ignored
}

TEST(RecipeTest, RemoveTags)
{
    Recipe recipe = createValidRecipe(2, "Tag Removal Test");
    recipe.addTag("Breakfast");
    recipe.addTag("Quick");
    recipe.addTag("Healthy");
    recipe.addTag("Quick"); // Add a duplicate for removal test
    ASSERT_EQ(recipe.getTags().size(), 4);

    // Remove existing tag (all instances)
    recipe.removeTag("Quick");
    ASSERT_EQ(recipe.getTags().size(), 2);
    EXPECT_FALSE(recipe.hasTag("Quick"));
    EXPECT_TRUE(recipe.hasTag("Breakfast"));
    EXPECT_TRUE(recipe.hasTag("Healthy"));

    // Remove non-existent tag
    recipe.removeTag("NonExistent");
    ASSERT_EQ(recipe.getTags().size(), 2);

    // Remove remaining tags
    recipe.removeTag("Breakfast");
    recipe.removeTag("Healthy");
    ASSERT_TRUE(recipe.getTags().empty());

    // Remove from empty
    recipe.removeTag("Anything");
    ASSERT_TRUE(recipe.getTags().empty());
}

TEST(RecipeTest, HasTag)
{
    Recipe recipe = createValidRecipe(3, "HasTag Test");
    EXPECT_FALSE(recipe.hasTag("AnyTag"));

    recipe.addTag("TestTag");
    EXPECT_TRUE(recipe.hasTag("TestTag"));
    EXPECT_FALSE(recipe.hasTag("testtag")); // Assuming case-sensitive
    EXPECT_FALSE(recipe.hasTag("OtherTag"));
}

TEST(RecipeTest, TagsInJsonSerialization)
{ // Already existed, ensure it covers tags
    Recipe originalRecipe(105, "Tags In JSON", {}, {}, 10, Difficulty::Easy, "Test");
    originalRecipe.addTag("TagA");
    originalRecipe.addTag("TagB");

    json recipeJson = originalRecipe;
    ASSERT_TRUE(recipeJson.contains("tags"));
    ASSERT_TRUE(recipeJson["tags"].is_array());
    ASSERT_EQ(recipeJson["tags"].size(), 2);

    bool tagAFound = false;
    bool tagBFound = false;
    for (const auto &item : recipeJson["tags"])
    {
        if (item.get<std::string>() == "TagA")
            tagAFound = true;
        if (item.get<std::string>() == "TagB")
            tagBFound = true;
    }
    EXPECT_TRUE(tagAFound);
    EXPECT_TRUE(tagBFound);

    Recipe deserializedRecipe = recipeJson.get<Recipe>();
    ASSERT_EQ(deserializedRecipe.getTags().size(), 2);
    EXPECT_TRUE(deserializedRecipe.hasTag("TagA"));
    EXPECT_TRUE(deserializedRecipe.hasTag("TagB"));
}

TEST(RecipeTest, TagsInJsonDeserialization_EmptyAndNull)
{ // Already existed, ensure it covers tags
    // Tags field missing
    json j_no_tags = {
        {"id", 106}, {"name", "No Tags Field"}, {"cuisine", "Test"}, {"cookingTime", 5}, {"difficulty", "Easy"}, {"ingredients", json::array()}, {"steps", json::array()}};
    Recipe recipe_no_tags = j_no_tags.get<Recipe>();
    EXPECT_TRUE(recipe_no_tags.getTags().empty());

    // Tags field is null
    json j_null_tags = {
        {"id", 107}, {"name", "Null Tags Field"}, {"cuisine", "Test"}, {"cookingTime", 5}, {"difficulty", "Easy"}, {"ingredients", json::array()}, {"steps", json::array()}, {"tags", nullptr}};
    Recipe recipe_null_tags = j_null_tags.get<Recipe>();
    EXPECT_TRUE(recipe_null_tags.getTags().empty());

    // Tags field is empty array
    json j_empty_array_tags = {
        {"id", 108}, {"name", "Empty Array Tags"}, {"cuisine", "Test"}, {"cookingTime", 5}, {"difficulty", "Easy"}, {"ingredients", json::array()}, {"steps", json::array()}, {"tags", json::array()}};
    Recipe recipe_empty_array_tags = j_empty_array_tags.get<Recipe>();
    EXPECT_TRUE(recipe_empty_array_tags.getTags().empty());
}

// It might be beneficial to add tests for ingredient/step parsing if that logic was complex,
// but since it's using nlohmann/json directly for vector<pair<string,string>> and vector<string>
// within the Recipe's to_json/from_json, those aspects are largely covered by the library's tests
// and our full serialization/deserialization test.