#include "gtest/gtest.h"
#include "domain/restaurant/Restaurant.h" // Adjusted path
#include <string>
#include <vector>
#include <algorithm> // For std::find

// Using namespace for convenience in tests
using namespace RecipeApp;

// Test fixture for Restaurant tests
class RestaurantTest : public ::testing::Test {
protected:
    // Restaurant::Builder restaurantBuilder_ = Restaurant::builder(0, "Default Name"); // Removed builder member
};

TEST_F(RestaurantTest, ConstructorAndGetters) {
    Restaurant r1 = Restaurant::builder(1, "Test Cafe")
                        .withAddress("123 Main St")
                        .withContact("555-1234")
                        .withOpeningHours("9am-5pm")
                        .build();
    EXPECT_EQ(r1.getRestaurantId(), 1);
    EXPECT_EQ(r1.getName(), "Test Cafe");
    EXPECT_EQ(r1.getAddress(), "123 Main St");
    EXPECT_EQ(r1.getContact(), "555-1234");
    EXPECT_EQ(r1.getOpeningHours(), "9am-5pm");
    EXPECT_TRUE(r1.getFeaturedRecipeIds().empty());
}

TEST_F(RestaurantTest, Setters) {
    Restaurant r2 = Restaurant::builder(2, "Old Name")
                        .withAddress("Old Address")
                        .withContact("Old Contact")
                        .withOpeningHours("Old Hours")
                        .build();
    r2.setName("New Cafe");
    r2.setAddress("456 Oak Ave");
    r2.setContact("555-5678");
    r2.setOpeningHours("10am-6pm");
    EXPECT_EQ(r2.getName(), "New Cafe");
    EXPECT_EQ(r2.getAddress(), "456 Oak Ave");
    EXPECT_EQ(r2.getContact(), "555-5678");
    EXPECT_EQ(r2.getOpeningHours(), "10am-6pm");
}

TEST_F(RestaurantTest, AddFeaturedRecipeIDs) {
    Restaurant r3 = Restaurant::builder(3, "Recipe Hub")
                        .withAddress("789 Pine Ln")
                        .withContact("555-9012")
                        .withOpeningHours("8am-8pm")
                        .build();
    r3.addFeaturedRecipe(101);
    r3.addFeaturedRecipe(102);
    r3.addFeaturedRecipe(101); // Add duplicate, should be ignored by current implementation
    
    const std::vector<int>& ids = r3.getFeaturedRecipeIds();
    // Assuming addFeaturedRecipe ensures uniqueness
    ASSERT_EQ(ids.size(), 2);
    // Check for presence, order might not be guaranteed by std::vector after additions/removals
    // if uniqueness is handled by searching and then pushing.
    // If addFeaturedRecipe uses a set internally or sorts, then order can be checked.
    // For now, let's check presence.
    EXPECT_NE(std::find(ids.begin(), ids.end(), 101), ids.end());
    EXPECT_NE(std::find(ids.begin(), ids.end(), 102), ids.end());
}

TEST_F(RestaurantTest, RemoveFeaturedRecipeIDs) {
    Restaurant r4 = Restaurant::builder(4, "Food Spot")
                        .withAddress("321 Elm St")
                        .withContact("555-3456")
                        .withOpeningHours("11am-9pm")
                        .build();
    r4.addFeaturedRecipe(201);
    r4.addFeaturedRecipe(202);
    r4.addFeaturedRecipe(203);

    r4.removeFeaturedRecipe(202); // Remove middle
    const std::vector<int>& ids1 = r4.getFeaturedRecipeIds();
    ASSERT_EQ(ids1.size(), 2);
    EXPECT_NE(std::find(ids1.begin(), ids1.end(), 201), ids1.end());
    EXPECT_NE(std::find(ids1.begin(), ids1.end(), 203), ids1.end());
    EXPECT_EQ(std::find(ids1.begin(), ids1.end(), 202), ids1.end());


    r4.removeFeaturedRecipe(204); // Remove non-existent
    const std::vector<int>& ids2 = r4.getFeaturedRecipeIds();
    ASSERT_EQ(ids2.size(), 2);

    r4.removeFeaturedRecipe(201); // Remove first (remaining)
    const std::vector<int>& ids3 = r4.getFeaturedRecipeIds();
    ASSERT_EQ(ids3.size(), 1);
    EXPECT_EQ(ids3[0], 203);
    
    r4.removeFeaturedRecipe(203); // Remove last (remaining)
    EXPECT_TRUE(r4.getFeaturedRecipeIds().empty());
}

// Placeholder for future JSON tests
// TEST_F(RestaurantTest, JsonSerialization) {}
// TEST_F(RestaurantTest, JsonDeserialization) {}

// Placeholder for future Builder specific tests
// TEST_F(RestaurantTest, BuilderMinimal) {}
// TEST_F(RestaurantTest, BuilderOverwrite) {}

// Placeholder for future equality operator tests
// TEST_F(RestaurantTest, EqualityOperator) {}
TEST_F(RestaurantTest, Builder_MinimalFields) {
    ASSERT_NO_THROW({
        Restaurant r = Restaurant::builder(10, "Minimal Resto")
                           .withAddress("Some Address") // Address is required by builder
                           .withContact("Some Contact") // Contact is required by builder
                           .build();
        EXPECT_EQ(r.getRestaurantId(), 10);
        EXPECT_EQ(r.getName(), "Minimal Resto");
        EXPECT_EQ(r.getAddress(), "Some Address");
        EXPECT_EQ(r.getContact(), "Some Contact");
        EXPECT_TRUE(r.getOpeningHours().empty()); // Default empty
        EXPECT_TRUE(r.getFeaturedRecipeIds().empty()); // Default empty
    });
}

TEST_F(RestaurantTest, Builder_OverwriteValues) {
    Restaurant r = Restaurant::builder(11, "Overwrite Resto")
                       .withAddress("Old Address")
                       .withAddress("New Address")
                       .withContact("Old Contact")
                       .withContact("New Contact")
                       .withOpeningHours("Old Hours")
                       .withOpeningHours("New Hours")
                       .withFeaturedRecipeIds({1, 2})
                       .withFeaturedRecipeIds({3, 4})
                       .build();
    EXPECT_EQ(r.getAddress(), "New Address");
    EXPECT_EQ(r.getContact(), "New Contact");
    EXPECT_EQ(r.getOpeningHours(), "New Hours");
    const auto& ids = r.getFeaturedRecipeIds();
    ASSERT_EQ(ids.size(), 2);
    EXPECT_EQ(ids[0], 3);
    EXPECT_EQ(ids[1], 4);
}

TEST_F(RestaurantTest, Setters_EmptyValues) {
    Restaurant r = Restaurant::builder(12, "Setter Test Resto")
                       .withAddress("123 Street")
                       .withContact("12345")
                       .build();
    
    EXPECT_THROW(r.setName(""), std::invalid_argument);
    EXPECT_THROW(r.setAddress(""), std::invalid_argument);
    EXPECT_THROW(r.setContact(""), std::invalid_argument);
    
    ASSERT_NO_THROW(r.setOpeningHours(""));
    EXPECT_TRUE(r.getOpeningHours().empty());

    ASSERT_NO_THROW(r.setFeaturedRecipeIds({}));
    EXPECT_TRUE(r.getFeaturedRecipeIds().empty());
}

TEST_F(RestaurantTest, AddFeaturedRecipe_InvalidIds) {
    Restaurant r = Restaurant::builder(13, "Invalid ID Resto")
                       .withAddress("Addr")
                       .withContact("Cont")
                       .build();
    // Assuming 0 or negative IDs are not explicitly blocked by addFeaturedRecipe itself,
    // but this might be a good place to test if such validation is desired.
    // For now, the method just adds them if not present.
    r.addFeaturedRecipe(0); 
    r.addFeaturedRecipe(-1);
    
    const auto& ids = r.getFeaturedRecipeIds();
    // If 0 and -1 are treated as valid distinct IDs by addFeaturedRecipe:
    // EXPECT_EQ(ids.size(), 2); 
    // However, if they are invalid and ignored, or if business logic elsewhere prevents this,
    // the test would need adjustment. Current addFeaturedRecipe only checks for duplicates.
    // Let's assume they are added if not duplicate.
    ASSERT_EQ(ids.size(), 2);
    EXPECT_NE(std::find(ids.begin(), ids.end(), 0), ids.end());
    EXPECT_NE(std::find(ids.begin(), ids.end(), -1), ids.end());
}

TEST_F(RestaurantTest, EqualityOperator) {
    Restaurant r1 = Restaurant::builder(20, "Resto A").withAddress("Addr1").withContact("Cont1").build();
    Restaurant r2 = Restaurant::builder(20, "Resto B").withAddress("Addr2").withContact("Cont2").build(); // Same ID
    Restaurant r3 = Restaurant::builder(21, "Resto C").withAddress("Addr3").withContact("Cont3").build(); // Different ID

    EXPECT_TRUE(r1 == r2);
    EXPECT_FALSE(r1 == r3);
}

// Basic JSON tests (more comprehensive tests would mock file I/O or use string streams)
TEST_F(RestaurantTest, JsonSerialization_Full) {
    Restaurant r_orig = Restaurant::builder(30, "JSON Full Resto")
                           .withAddress("789 Json Rd")
                           .withContact("555-JSON")
                           .withOpeningHours("24/7")
                           .withFeaturedRecipeIds({10, 20, 30})
                           .build();
    
    nlohmann::json j = r_orig; // Uses to_json

    EXPECT_EQ(j["id"], 30);
    EXPECT_EQ(j["name"], "JSON Full Resto");
    EXPECT_EQ(j["address"], "789 Json Rd");
    EXPECT_EQ(j["contact"], "555-JSON");
    EXPECT_EQ(j["openingHours"], "24/7");
    ASSERT_TRUE(j["featuredRecipeIds"].is_array());
    EXPECT_EQ(j["featuredRecipeIds"].size(), 3);
    EXPECT_EQ(j["featuredRecipeIds"][0], 10);
}

TEST_F(RestaurantTest, JsonDeserialization_Full) {
    nlohmann::json j = {
        {"id", 31},
        {"name", "JSON Deserialize Resto"},
        {"address", "101 Binary Ave"},
        {"contact", "555-BITS"},
        {"openingHours", "10am-10pm"},
        {"featuredRecipeIds", {11, 22}}
    };

    Restaurant r = j.get<Restaurant>(); // Uses from_json

    EXPECT_EQ(r.getRestaurantId(), 31);
    EXPECT_EQ(r.getName(), "JSON Deserialize Resto");
    EXPECT_EQ(r.getAddress(), "101 Binary Ave");
    EXPECT_EQ(r.getContact(), "555-BITS");
    EXPECT_EQ(r.getOpeningHours(), "10am-10pm");
    const auto& ids = r.getFeaturedRecipeIds();
    ASSERT_EQ(ids.size(), 2);
    EXPECT_EQ(ids[0], 11);
}

TEST_F(RestaurantTest, JsonDeserialization_OptionalFieldsMissing) {
    nlohmann::json j = {
        {"id", 32},
        {"name", "Minimal JSON Resto"},
        {"address", "Min Address"},
        {"contact", "Min Contact"}
        // openingHours and featuredRecipeIds are missing
    };
    Restaurant r = j.get<Restaurant>();
    EXPECT_TRUE(r.getOpeningHours().empty());
    EXPECT_TRUE(r.getFeaturedRecipeIds().empty());
}

TEST_F(RestaurantTest, JsonDeserialization_OptionalFieldsNull) {
     nlohmann::json j = {
        {"id", 33},
        {"name", "Null JSON Resto"},
        {"address", "Null Address"},
        {"contact", "Null Contact"},
        {"openingHours", nullptr},
        {"featuredRecipeIds", nullptr}
    };
    // The current from_json for Restaurant expects openingHours to be a string if present,
    // and featuredRecipeIds to be an array if present.
    // nlohmann::json will convert nullptr to an empty string for string fields if not handled explicitly,
    // and for arrays, it might become an empty array or cause issues depending on get<std::vector<int>>.
    // Let's assume the from_json handles nullptr for optional string (openingHours) by making it empty,
    // and for optional vector (featuredRecipeIds) by making it empty.
    // If from_json throws for nullptr on these, this test needs adjustment or from_json needs to be more robust.
    
    // For openingHours: if j.at("openingHours").is_string() is false (because it's null), it won't be set.
    // For featuredRecipeIds: if j.at("featuredRecipeIds").is_array() is false, it won't be set.
    // So they should remain default (empty).
    Restaurant r = j.get<Restaurant>();
    EXPECT_TRUE(r.getOpeningHours().empty());
    EXPECT_TRUE(r.getFeaturedRecipeIds().empty());
}

TEST_F(RestaurantTest, JsonDeserialization_InvalidData) {
    // Missing required fields
    EXPECT_THROW(nlohmann::json({{"name", "No ID"}}).get<Restaurant>(), std::runtime_error);
    EXPECT_THROW(nlohmann::json({{"id", 1}}).get<Restaurant>(), std::runtime_error); // Missing name, address, contact
    EXPECT_THROW(nlohmann::json({{"id", 1}, {"name", "No Addr"}}).get<Restaurant>(), std::runtime_error); // Missing address, contact
    EXPECT_THROW(nlohmann::json({{"id", 1}, {"name", "No Contact"}, {"address", "Addr"}}).get<Restaurant>(), std::runtime_error); // Missing contact

    // Invalid types
    EXPECT_THROW(nlohmann::json({{"id", "not-an-int"}, {"name", "N"}, {"address", "A"}, {"contact", "C"}}).get<Restaurant>(), std::runtime_error);
    EXPECT_THROW(nlohmann::json({{"id", 1}, {"name", 123}, {"address", "A"}, {"contact", "C"}}).get<Restaurant>(), std::runtime_error);
}