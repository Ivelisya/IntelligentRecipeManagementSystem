#include <algorithm>  // For std::find in hasTag
#include <stdexcept>  // For std::invalid_argument, std::runtime_error

#include "domain/recipe/Recipe.h"
#include "gtest/gtest.h"
#include "json.hpp"  // For nlohmann::json for testing serialization

// Convenience alias
using json = nlohmann::json;
using namespace RecipeApp;

// Helper function to create a basic valid Recipe object for tests
Recipe createValidRecipe(int id = 1, const std::string &name = "Test Recipe") {
    return Recipe::builder(id, name)
        .withIngredients(
            {Ingredient{"Ingredient1", "1 cup"}})  // Use Ingredient struct
        .withSteps({"Step 1"})
        .withCookingTime(30)
        .withDifficulty(Difficulty::Medium)
        .withTags({"Test Cuisine"})  // Use tags
        .build();
}

TEST(RecipeTest, ConstructorValidations) {
    // Test valid construction
    ASSERT_NO_THROW(Recipe::builder(1, "Pasta Carbonara")
                        .withIngredients({Ingredient{"Spaghetti", "200g"},
                                          Ingredient{"Egg Yolk", "2"},
                                          Ingredient{"Pancetta", "100g"},
                                          Ingredient{"Parmesan", "50g"}})
                        .withSteps({"Cook spaghetti.", "Fry pancetta.",
                                    "Mix eggs and cheese.", "Combine all."})
                        .withCookingTime(25)
                        .withDifficulty(Difficulty::Medium)
                        .withTags({"Italian"})  // Use tags
                        .build());

    // Test invalid cooking time (negative) - Builder will throw
    ASSERT_THROW(Recipe::builder(2, "Bad Recipe")
                     .withIngredients({})
                     .withSteps({})
                     .withCookingTime(-10)  // This will throw
                     .withDifficulty(Difficulty::Easy)
                     .withTags({"None"})
                     .build(),
                 std::invalid_argument);
}

TEST(RecipeTest, SettersValidations) {
    Recipe recipe = createValidRecipe();

    // Name
    ASSERT_NO_THROW(recipe.setName("New Valid Name"));
    EXPECT_EQ(recipe.getName(), "New Valid Name");
    ASSERT_THROW(recipe.setName(""), std::invalid_argument);

    // Cooking Time
    ASSERT_NO_THROW(recipe.setCookingTime(45));
    EXPECT_EQ(recipe.getCookingTime(), 45);
    ASSERT_THROW(recipe.setCookingTime(-5), std::invalid_argument);
    EXPECT_EQ(recipe.getCookingTime(), 45);  // Should remain unchanged

    // Tags (replacing Cuisine)
    ASSERT_NO_THROW(recipe.setTags({"New Tag"}));
    EXPECT_TRUE(recipe.hasTag("New Tag"));
    // ASSERT_THROW(recipe.setTags({""}), std::invalid_argument); // Setting
    // empty tag in vector is valid, addTag might ignore empty string

    // Difficulty
    recipe.setDifficulty(Difficulty::Hard);
    EXPECT_EQ(recipe.getDifficulty(), Difficulty::Hard);

    // Ingredients
    std::vector<Ingredient> newIngredientsStruct = {
        Ingredient{"New Ing", "1"}, Ingredient{"Another Ing", "2kg"}};
    recipe.setIngredients(newIngredientsStruct);
    EXPECT_EQ(recipe.getIngredients().size(), 2);
    EXPECT_EQ(recipe.getIngredients()[0].name, "New Ing");  // Use .name

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

TEST(RecipeTest, Getters) {
    Recipe recipe = Recipe::builder(10, "Getter Test")
                        .withIngredients({Ingredient{"Water", "1L"}})
                        .withSteps({"Boil"})
                        .withCookingTime(5)
                        .withDifficulty(Difficulty::Easy)
                        .withTags({"Testing"})  // Use tags
                        .build();
    recipe.setNutritionalInfo("Test Nutrition");
    recipe.setImageUrl("test.png");

    EXPECT_EQ(recipe.getRecipeId(), 10);
    EXPECT_EQ(recipe.getName(), "Getter Test");
    EXPECT_EQ(recipe.getIngredients().size(), 1);
    EXPECT_EQ(recipe.getIngredients()[0].name, "Water");  // Use .name
    EXPECT_EQ(recipe.getSteps().size(), 1);
    EXPECT_EQ(recipe.getSteps()[0], "Boil");
    EXPECT_EQ(recipe.getCookingTime(), 5);
    EXPECT_EQ(recipe.getDifficulty(), Difficulty::Easy);
    EXPECT_TRUE(recipe.hasTag("Testing"));  // Check tag
    ASSERT_TRUE(recipe.getNutritionalInfo().has_value());
    EXPECT_EQ(recipe.getNutritionalInfo().value(), "Test Nutrition");
    ASSERT_TRUE(recipe.getImageUrl().has_value());
    EXPECT_EQ(recipe.getImageUrl().value(), "test.png");
}

TEST(RecipeTest, GetDisplayDetails) {
    Recipe recipe = createValidRecipe(1, "Detailed Recipe");
    recipe.setNutritionalInfo("Info");
    recipe.setImageUrl("url");

    std::string details = recipe.getDisplayDetails();
    EXPECT_NE(details.find("Recipe ID: 1"), std::string::npos);
    EXPECT_NE(details.find("Name: Detailed Recipe"), std::string::npos);
    // EXPECT_NE(details.find("Cuisine: Test Cuisine"), std::string::npos); //
    // Cuisine removed, check for Tags if displayDetails is updated
    EXPECT_NE(details.find("Tags: Test Cuisine"),
              std::string::npos);  // Assuming getDisplayDetails shows tags
    EXPECT_NE(details.find("Difficulty: Medium"), std::string::npos);
    EXPECT_NE(details.find("Cooking Time: 30 minutes"), std::string::npos);
    EXPECT_NE(details.find("Ingredient1: 1 cup"), std::string::npos);
    EXPECT_NE(details.find("1. Step 1"), std::string::npos);
    EXPECT_NE(details.find("Nutritional Info: Info"), std::string::npos);
    EXPECT_NE(details.find("Image URL: url"), std::string::npos);
}

TEST(RecipeTest, JsonSerializationDeserializationFull) {
    Recipe originalRecipe =
        Recipe::builder(101, "Full JSON Test")
            .withIngredients(
                {Ingredient{"Flour", "500g"}, Ingredient{"Sugar", "200g"}})
            .withSteps({"Mix dry ingredients", "Add wet ingredients", "Bake"})
            .withCookingTime(60)
            .withDifficulty(Difficulty::Hard)
            .withTags({"Bakery", "Dessert", "Cake"})  // Use tags
            .withNutritionalInfo("High Sugar")
            .withImageUrl("http://baking.com/cake.jpg")
            .build();
    // originalRecipe.addTag("Dessert"); // Tags added via builder
    // originalRecipe.addTag("Cake");

    // Serialize
    json recipeJson = originalRecipe;  // Uses to_json

    // Check some key fields in JSON
    ASSERT_EQ(recipeJson["id"], 101);
    ASSERT_EQ(recipeJson["name"], "Full JSON Test");
    // ASSERT_EQ(recipeJson["cuisine"], "Bakery"); // Cuisine removed
    ASSERT_EQ(recipeJson["cookingTime"], 60);
    ASSERT_EQ(recipeJson["difficulty"], "Hard");  // Serialized as string
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
    ASSERT_EQ(recipeJson["tags"].size(), 3);  // Bakery, Dessert, Cake
    // EXPECT_EQ(recipeJson["tags"][0], "Dessert"); // Order might not be
    // guaranteed, check for presence EXPECT_EQ(recipeJson["tags"][1], "Cake");

    // Deserialize
    Recipe deserializedRecipe = recipeJson.get<Recipe>();  // Uses from_json

    EXPECT_EQ(deserializedRecipe.getRecipeId(), originalRecipe.getRecipeId());
    EXPECT_EQ(deserializedRecipe.getName(), originalRecipe.getName());
    // EXPECT_EQ(deserializedRecipe.getCuisine(), originalRecipe.getCuisine());
    // // Cuisine removed
    EXPECT_EQ(deserializedRecipe.getCookingTime(),
              originalRecipe.getCookingTime());
    EXPECT_EQ(deserializedRecipe.getDifficulty(),
              originalRecipe.getDifficulty());
    ASSERT_EQ(deserializedRecipe.getIngredients().size(),
              originalRecipe.getIngredients().size());
    EXPECT_EQ(deserializedRecipe.getIngredients()[0].name,
              originalRecipe.getIngredients()[0].name);  // Use .name
    ASSERT_EQ(deserializedRecipe.getSteps().size(),
              originalRecipe.getSteps().size());
    EXPECT_EQ(deserializedRecipe.getSteps()[0], originalRecipe.getSteps()[0]);
    ASSERT_TRUE(deserializedRecipe.getNutritionalInfo().has_value());
    EXPECT_EQ(deserializedRecipe.getNutritionalInfo().value(),
              originalRecipe.getNutritionalInfo().value());
    ASSERT_TRUE(deserializedRecipe.getImageUrl().has_value());
    EXPECT_EQ(deserializedRecipe.getImageUrl().value(),
              originalRecipe.getImageUrl().value());
    ASSERT_EQ(deserializedRecipe.getTags().size(), 3);
    EXPECT_TRUE(deserializedRecipe.hasTag("Bakery"));
    EXPECT_TRUE(deserializedRecipe.hasTag("Dessert"));
    EXPECT_TRUE(deserializedRecipe.hasTag("Cake"));
}

TEST(RecipeTest, JsonDeserializationOptionalFieldsMissing) {
    json j = {
        {"id", 102},
        {"name", "Minimal Recipe"},
        // {"cuisine", "Minimalist"}, // Cuisine removed
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
    EXPECT_TRUE(
        recipe.getTags().empty());  // Tags should be empty if missing in JSON
}

TEST(RecipeTest, JsonDeserializationOptionalFieldsNull) {
    json j = {
        {"id", 103},
        {"name", "Null Optionals Recipe"},
        // {"cuisine", "Nullable"}, // Cuisine removed
        {"cookingTime", 15},
        {"difficulty", "Medium"},
        {"ingredients", json::array()},  // Keep it simple for this test
        {"steps", json::array()},
        {"nutritionalInfo", nullptr},
        {"imageUrl", nullptr},
        {"tags", nullptr}  // Test tags as null
    };
    j["ingredients"].push_back({{"name", "item"}, {"quantity", "1"}});

    Recipe recipe = j.get<Recipe>();

    EXPECT_EQ(recipe.getRecipeId(), 103);
    EXPECT_FALSE(recipe.getNutritionalInfo().has_value());
    EXPECT_FALSE(recipe.getImageUrl().has_value());
    EXPECT_TRUE(
        recipe.getTags().empty());  // Tags should be empty if null in JSON
}

TEST(RecipeTest, JsonDeserializationInvalidData) {
    // Missing ID
    json j_no_id = {{"name", "No ID Recipe"},
                    // {"cuisine", "Invalid"}, // Cuisine removed
                    {"cookingTime", 10},
                    {"difficulty", "Easy"}};
    ASSERT_THROW(j_no_id.get<Recipe>(), std::runtime_error);

    // Invalid ID (zero)
    json j_zero_id = {{"id", 0},
                      {"name", "Zero ID Recipe"},
                      // {"cuisine", "Invalid"}, // Cuisine removed
                      {"cookingTime", 10},
                      {"difficulty", "Easy"}};
    ASSERT_THROW(j_zero_id.get<Recipe>(), std::runtime_error);

    // Missing Name
    json j_no_name = {{"id", 201},
                      // {"cuisine", "Invalid"}, // Cuisine removed
                      {"cookingTime", 10},
                      {"difficulty", "Easy"}};
    ASSERT_THROW(j_no_name.get<Recipe>(), std::runtime_error);

    // Empty Name
    json j_empty_name = {{"id", 202},
                         {"name", ""},
                         // {"cuisine", "Invalid"}, // Cuisine removed
                         {"cookingTime", 10},
                         {"difficulty", "Easy"}};
    ASSERT_THROW(j_empty_name.get<Recipe>(), std::runtime_error);

    // Missing cookingTime
    json j_no_time = {{"id", 203},
                      {"name", "No Time Recipe"},
                      // {"cuisine", "Invalid"}, // Cuisine removed
                      {"difficulty", "Easy"}};
    ASSERT_THROW(j_no_time.get<Recipe>(), std::runtime_error);

    // Negative cookingTime
    json j_neg_time = {{"id", 204},
                       {"name", "Negative Time Recipe"},
                       // {"cuisine", "Invalid"}, // Cuisine removed
                       {"cookingTime", -10},
                       {"difficulty", "Easy"}};
    ASSERT_THROW(j_neg_time.get<Recipe>(), std::runtime_error);

    // Missing difficulty
    json j_no_difficulty = {{"id", 205},
                            {"name", "No Difficulty Recipe"},
                            // {"cuisine", "Invalid"}, // Cuisine removed
                            {"cookingTime", 10}};
    ASSERT_THROW(j_no_difficulty.get<Recipe>(), std::runtime_error);

    // Invalid difficulty string
    json j_invalid_difficulty = {
        {"id", 206},
        {"name", "Invalid Diff Recipe"},
        // {"cuisine", "Valid"}, // Cuisine removed
        {"cookingTime", 10},
        {"difficulty", "SuperHard"}  // Not a valid enum string
    };
    Recipe recipe_invalid_diff = j_invalid_difficulty.get<Recipe>();
    EXPECT_EQ(recipe_invalid_diff.getDifficulty(),
              RecipeApp::Difficulty::Easy);  // nlohmann::json defaults to first
                                             // enum value on parse error
}

TEST(RecipeTest, EqualityOperator) {
    Recipe recipe1 = createValidRecipe(1, "Recipe One");
    Recipe recipe2 =
        createValidRecipe(1, "Recipe One Variant");  // Same ID, different name
    Recipe recipe3 = createValidRecipe(2, "Recipe Two");  // Different ID

    EXPECT_TRUE(recipe1 == recipe2);  // Equality is based on ID only
    EXPECT_FALSE(recipe1 == recipe3);
    EXPECT_FALSE(recipe2 == recipe3);
}

// --- Tag Management Tests ---
TEST(RecipeTest, AddAndGetTags) {
    Recipe recipe = Recipe::builder(1, "Tag Test Recipe")
                        .withIngredients({Ingredient{"Flour", "1 cup"}})
                        .withSteps({"Mix"})
                        .withCookingTime(10)
                        .withDifficulty(Difficulty::Easy)
                        // No .withTags() here initially
                        .build();
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
    recipe.addTag(
        "Easy");  // Should ideally not add if already present, or
                  // Recipe::addTag handles uniqueness Current Recipe::addTag
                  // allows duplicates, so size will be 3. If addTag is changed
                  // to enforce uniqueness, this assertion needs to be 2.
    // Let's assume current behavior allows duplicates for this test, then check
    // for specific unique tag presence. To test uniqueness, we'd check size
    // before and after, or use a set-like structure internally in Recipe. For
    // now, we'll stick to hasTag for verification of presence.
    size_t easy_count = 0;
    for (const auto &t : recipe.getTags())
        if (t == "Easy") easy_count++;
    EXPECT_GE(easy_count, 1);  // At least one "Easy" tag
    EXPECT_EQ(recipe.getTags().size(),
              2);  // Now expects 2 due to uniqueness

    // Adding empty tag
    recipe.addTag("");
    // Assuming Recipe::addTag ignores empty strings based on its implementation
    bool foundEmpty = false;
    for (const auto &tag : recipe.getTags()) {
        if (tag.empty()) foundEmpty = true;
    }
    EXPECT_FALSE(foundEmpty);
    EXPECT_EQ(recipe.getTags().size(),
              2);  // Size should not change if empty tags are ignored, and was 2

}

TEST(RecipeTest, RemoveTags) {
    Recipe recipe = Recipe::builder(2, "Tag Removal Test")
                        .withIngredients({Ingredient{"Water", "1L"}})
                        .withSteps({"Boil"})
                        .withCookingTime(5)
                        .withDifficulty(Difficulty::Easy)
                        // No .withTags() here initially
                        .build();
    ASSERT_TRUE(recipe.getTags().empty());  // Should be empty initially

    recipe.addTag("Breakfast");
    recipe.addTag("Quick");
    recipe.addTag("Healthy");
    recipe.addTag("Quick");                 // Add a duplicate, should be ignored
    ASSERT_EQ(recipe.getTags().size(), 3);  // Expect 3 unique tags

    // Remove existing tag (all instances)
    recipe.removeTag("Quick");
    // Tags should be {"Breakfast", "Healthy"}
    ASSERT_EQ(recipe.getTags().size(), 2);  // This should be correct now
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

TEST(RecipeTest, HasTag) {
    Recipe recipe = createValidRecipe(3, "HasTag Test");
    EXPECT_FALSE(recipe.hasTag("AnyTag"));

    recipe.addTag("TestTag");
    EXPECT_TRUE(recipe.hasTag("TestTag"));
    EXPECT_FALSE(recipe.hasTag("testtag"));  // Assuming case-sensitive
    EXPECT_FALSE(recipe.hasTag("OtherTag"));
}

TEST(RecipeTest, TagsInJsonSerialization) {
    Recipe originalRecipe =
        Recipe::builder(105, "Tags In JSON")
            .withIngredients({})
            .withSteps({})
            .withCookingTime(10)
            .withDifficulty(Difficulty::Easy)
            .withTags(
                {"Test", "TagA", "TagB"})  // Add "Test" as a tag, and others
            .build();
    // originalRecipe.addTag("TagA"); // Added via builder
    // originalRecipe.addTag("TagB");

    json recipeJson = originalRecipe;
    ASSERT_TRUE(recipeJson.contains("tags"));
    ASSERT_TRUE(recipeJson["tags"].is_array());
    ASSERT_EQ(recipeJson["tags"].size(), 3);  // Test, TagA, TagB

    bool tagAFound = false;
    bool tagBFound = false;
    bool testTagFound = false;
    for (const auto &item : recipeJson["tags"]) {
        if (item.get<std::string>() == "TagA") tagAFound = true;
        if (item.get<std::string>() == "TagB") tagBFound = true;
        if (item.get<std::string>() == "Test") testTagFound = true;
    }
    EXPECT_TRUE(tagAFound);
    EXPECT_TRUE(tagBFound);
    EXPECT_TRUE(testTagFound);

    Recipe deserializedRecipe = recipeJson.get<Recipe>();
    ASSERT_EQ(deserializedRecipe.getTags().size(), 3);
    EXPECT_TRUE(deserializedRecipe.hasTag("TagA"));
    EXPECT_TRUE(deserializedRecipe.hasTag("TagB"));
}

TEST(RecipeTest,
     TagsInJsonDeserialization_EmptyAndNull) {  // Already existed, ensure it
                                                // covers tags
    // Tags field missing
    json j_no_tags = {{"id", 106},
                      {"name", "No Tags Field"},
                      /*{"cuisine", "Test"},*/ {"cookingTime", 5},
                      {"difficulty", "Easy"},
                      {"ingredients", json::array()},
                      {"steps", json::array()}};
    Recipe recipe_no_tags = j_no_tags.get<Recipe>();
    EXPECT_TRUE(recipe_no_tags.getTags().empty());

    // Tags field is null
    json j_null_tags = {{"id", 107},
                        {"name", "Null Tags Field"},
                        /*{"cuisine", "Test"},*/ {"cookingTime", 5},
                        {"difficulty", "Easy"},
                        {"ingredients", json::array()},
                        {"steps", json::array()},
                        {"tags", nullptr}};
    Recipe recipe_null_tags = j_null_tags.get<Recipe>();
    EXPECT_TRUE(recipe_null_tags.getTags().empty());

    // Tags field is empty array
    json j_empty_array_tags = {{"id", 108},
                               {"name", "Empty Array Tags"},
                               /*{"cuisine", "Test"},*/ {"cookingTime", 5},
                               {"difficulty", "Easy"},
                               {"ingredients", json::array()},
                               {"steps", json::array()},
                               {"tags", json::array()}};
    Recipe recipe_empty_array_tags = j_empty_array_tags.get<Recipe>();
    EXPECT_TRUE(recipe_empty_array_tags.getTags().empty());
}

// It might be beneficial to add tests for ingredient/step parsing if that logic
// was complex, but since it's using nlohmann/json directly for
// vector<pair<string,string>> and vector<string> within the Recipe's
// to_json/from_json, those aspects are largely covered by the library's tests
// and our full serialization/deserialization test.
// --- Additional Builder Tests ---
TEST(RecipeBuilderTest, BuildWithOnlyMandatoryFields) {
    ASSERT_NO_THROW({
        Recipe recipe = Recipe::builder(200, "Minimal Recipe Build")
                            // No other .withXxx calls
                            .build();
        EXPECT_EQ(recipe.getRecipeId(), 200);
        EXPECT_EQ(recipe.getName(), "Minimal Recipe Build");
        EXPECT_TRUE(recipe.getIngredients().empty());
        EXPECT_TRUE(recipe.getSteps().empty());
        EXPECT_EQ(recipe.getCookingTime(), 0); // Assuming 0 is default
        EXPECT_EQ(recipe.getDifficulty(), RecipeApp::Difficulty::Easy); // Default is Easy as per RecipeBuilder
        EXPECT_TRUE(recipe.getTags().empty());
        EXPECT_FALSE(recipe.getNutritionalInfo().has_value());
        EXPECT_FALSE(recipe.getImageUrl().has_value());
    });
}

TEST(RecipeBuilderTest, BuilderMethodOverwriting) {
    ASSERT_NO_THROW({
        Recipe recipe = Recipe::builder(201, "Overwrite Test")
                            .withCookingTime(10)
                            .withCookingTime(20) // This should overwrite the previous
                            .withDifficulty(Difficulty::Easy)
                            .withDifficulty(Difficulty::Hard) // Overwrite
                            .withTags({"InitialTag"})
                            .withTags({"FinalTag"}) // Overwrite
                            .withIngredients({Ingredient{"OldIng", "1"}})
                            .withIngredients({Ingredient{"NewIng", "2"}}) // Overwrite
                            .withSteps({"OldStep"})
                            .withSteps({"NewStep"}) // Overwrite
                            .withNutritionalInfo("OldInfo")
                            .withNutritionalInfo("NewInfo") // Overwrite
                            .withImageUrl("old.url")
                            .withImageUrl("new.url") // Overwrite
                            .build();

        EXPECT_EQ(recipe.getCookingTime(), 20);
        EXPECT_EQ(recipe.getDifficulty(), Difficulty::Hard);
        ASSERT_EQ(recipe.getTags().size(), 1);
        EXPECT_EQ(recipe.getTags()[0], "FinalTag");
        ASSERT_EQ(recipe.getIngredients().size(), 1);
        EXPECT_EQ(recipe.getIngredients()[0].name, "NewIng");
        ASSERT_EQ(recipe.getSteps().size(), 1);
        EXPECT_EQ(recipe.getSteps()[0], "NewStep");
        ASSERT_TRUE(recipe.getNutritionalInfo().has_value());
        EXPECT_EQ(recipe.getNutritionalInfo().value(), "NewInfo");
        ASSERT_TRUE(recipe.getImageUrl().has_value());
        EXPECT_EQ(recipe.getImageUrl().value(), "new.url");
    });
}

TEST(RecipeTest, AddTagEnforcesUniquenessAndIgnoresEmpty) {
    Recipe recipe = Recipe::builder(1, "Tag Uniqueness Test")
                        .withIngredients({})
                        .withSteps({})
                        .build();
    
    recipe.addTag("Fruit");
    ASSERT_EQ(recipe.getTags().size(), 1);
    EXPECT_TRUE(recipe.hasTag("Fruit"));

    recipe.addTag("Fruit"); // Add duplicate
    ASSERT_EQ(recipe.getTags().size(), 1); // Size should remain 1 if unique

    recipe.addTag(""); // Add empty tag
    ASSERT_EQ(recipe.getTags().size(), 1); // Size should remain 1 if empty is ignored
    EXPECT_FALSE(recipe.hasTag(""));

    recipe.addTag("Vegetable");
    ASSERT_EQ(recipe.getTags().size(), 2);
    EXPECT_TRUE(recipe.hasTag("Vegetable"));
}