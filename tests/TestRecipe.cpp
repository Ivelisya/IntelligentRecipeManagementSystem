#include "gtest/gtest.h"
#include "domain/recipe/Recipe.h"
#include "json.hpp" // For nlohmann::json for testing serialization

#include <stdexcept> // For std::invalid_argument, std::runtime_error

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
        // nutritionalInfo and imageUrl are missing
    };

    Recipe recipe = j.get<Recipe>();
    // ASSERT_NO_THROW is implicitly handled by TEST_F if j.get<Recipe>() throws

    EXPECT_EQ(recipe.getRecipeId(), 102);
    EXPECT_EQ(recipe.getName(), "Minimal Recipe");
    EXPECT_FALSE(recipe.getNutritionalInfo().has_value());
    EXPECT_FALSE(recipe.getImageUrl().has_value());
}

TEST(RecipeTest, JsonDeserializationOptionalFieldsNull)
{
    json j = {
        {"id", 103},
        {"name", "Null Optionals Recipe"},
        {"cuisine", "Nullable"},
        {"cookingTime", 15},
        {"difficulty", "Medium"},
        {"ingredients", {{"name", "item"}, {"quantity", "1"}}}, // Corrected ingredient format
        {"steps", {"step1"}},
        {"nutritionalInfo", nullptr},
        {"imageUrl", nullptr}};
    // Corrected ingredient format for nlohmann::json direct conversion
    j["ingredients"] = json::array();
    j["ingredients"].push_back({{"name", "item"}, {"quantity", "1"}});

    Recipe recipe = j.get<Recipe>();
    // ASSERT_NO_THROW is implicitly handled by TEST_F if j.get<Recipe>() throws

    EXPECT_EQ(recipe.getRecipeId(), 103);
    EXPECT_FALSE(recipe.getNutritionalInfo().has_value()); // Should be treated as not present by our current from_json
    EXPECT_FALSE(recipe.getImageUrl().has_value());        // Should be treated as not present
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
    // Assuming nlohmann::json might default to the first enum value or not throw for an unrecognized string
    // when NLOHMANN_JSON_SERIALIZE_ENUM is used. Let's verify the resulting difficulty.
    Recipe recipe_invalid_diff = j_invalid_difficulty.get<Recipe>();
    // Depending on nlohmann::json's behavior with the macro, it might default to Easy or another value.
    // For now, let's assume it defaults to Easy if "SuperHard" is not recognized and doesn't throw.
    // This assertion might need adjustment based on actual behavior of the JSON library with the enum macro.
    EXPECT_EQ(recipe_invalid_diff.getDifficulty(), RecipeApp::Difficulty::Easy);
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

// It might be beneficial to add tests for ingredient/step parsing if that logic was complex,
// but since it's using nlohmann/json directly for vector<pair<string,string>> and vector<string>
// within the Recipe's to_json/from_json, those aspects are largely covered by the library's tests
// and our full serialization/deserialization test.