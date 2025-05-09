#include "CliUtils.h"
#include <iostream>
#include <limits>    // Required for std::numeric_limits
#include <stdexcept> // Required for std::exception
#include <vector>
#include <string>
#include <utility> // For std::pair

// Note: domain/recipe/Recipe.h and domain/user/User.h are already included via CliUtils.h

namespace RecipeApp
{
    namespace CliUtils
    {
        // Definition for verbose mode status
        static bool verbose_mode = false;

        void setVerbose(bool verbose)
        {
            verbose_mode = verbose;
        }

        bool isVerbose()
        {
            return verbose_mode;
        }

        std::string getPasswordFromConsole(const std::string &prompt)
        {
            std::string password;
            std::cout << prompt;
            // WARNING: Plain text password input - not secure for production
            std::cin >> password;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return password;
        }

        std::string getStringFromConsole(const std::string &prompt)
        {
            std::string input;
            std::cout << prompt;
            std::getline(std::cin, input);
            return input;
        }

        int getIntFromConsole(const std::string &prompt)
        {
            int value;
            while (true)
            {
                std::cout << prompt;
                std::string line;
                std::getline(std::cin, line);
                if (line.empty())
                {
                    std::cout << "Input cannot be empty. Please enter an integer." << std::endl;
                    continue;
                }
                try
                {
                    size_t processed_chars;
                    value = std::stoi(line, &processed_chars);
                    if (processed_chars == line.length())
                    {
                        break;
                    }
                    else
                    {
                        std::cout << "Invalid input. Please enter a pure integer." << std::endl;
                    }
                }
                catch (const std::invalid_argument &ia)
                {
                    std::cout << "Invalid input. Please enter an integer." << std::endl;
                }
                catch (const std::out_of_range &oor)
                {
                    std::cout << "Input number is out of range." << std::endl;
                }
            }
            return value;
        }

        RecipeApp::UserRole getRoleSelectionFromConsole()
        {
            int choice = 0;
            while (true)
            {
                std::cout << "Please select user role:" << std::endl;
                std::cout << "1. Normal User (Normal)" << std::endl;
                std::cout << "2. Administrator (Admin)" << std::endl;
                choice = getIntFromConsole("Enter option (1-2): ");
                if (choice == 1 || choice == 2)
                    break;
                std::cout << "Invalid option. Please re-enter." << std::endl;
            }
            return (choice == 1) ? RecipeApp::UserRole::Normal : RecipeApp::UserRole::Admin;
        }

        void displayUserDetailsBrief(const RecipeApp::User &user)
        {
            std::cout << "  ID: " << user.getUserId()
                      << ", Username: " << user.getUsername()
                      << ", Role: " << (user.getRole() == RecipeApp::UserRole::Admin ? "Administrator" : "Normal User")
                      << std::endl;
        }

        RecipeApp::Difficulty getDifficultyFromConsole()
        {
            int choice = 0;
            while (true)
            {
                std::cout << "Please select difficulty level:" << std::endl;
                std::cout << "1. Easy" << std::endl;
                std::cout << "2. Medium" << std::endl;
                std::cout << "3. Hard" << std::endl;
                choice = getIntFromConsole("Enter option (1-3): ");
                if (choice >= 1 && choice <= 3)
                {
                    break;
                }
                std::cout << "Invalid option. Please re-enter." << std::endl;
            }
            switch (choice)
            {
            case 1:
                return RecipeApp::Difficulty::Easy;
            case 2:
                return RecipeApp::Difficulty::Medium;
            case 3:
                return RecipeApp::Difficulty::Hard;
            default:
                // Should not reach here due to loop condition, but good practice for switch
                std::cerr << "Warning: getDifficultyFromConsole reached default case." << std::endl;
                return RecipeApp::Difficulty::Easy;
            }
        }

        std::vector<std::pair<std::string, std::string>> getIngredientsFromConsole()
        {
            std::vector<std::pair<std::string, std::string>> ingredients;
            std::cout << "Enter ingredients (one per line, format: [Ingredient Name] [Quantity and Unit], e.g., Eggs 2pcs. Type 'done' or empty line to finish):" << std::endl;
            std::string line;
            while (true)
            {
                std::cout << "Ingredient> ";
                std::getline(std::cin, line);
                if (line == "done" || line == "DONE" || line.empty())
                {
                    if (ingredients.empty() && line.empty())
                    {
                        std::string confirmEnd = getStringFromConsole("No ingredients entered. Are you sure you want to finish? (y/n): ");
                        if (confirmEnd != "y" && confirmEnd != "Y")
                            continue;
                    }
                    break;
                }
                size_t lastSpace = line.find_last_of(" \t");
                std::string name, quantity;
                if (lastSpace != std::string::npos && lastSpace > 0 && lastSpace < line.length() - 1)
                {
                    name = line.substr(0, lastSpace);
                    quantity = line.substr(lastSpace + 1);
                }
                else
                {
                    name = line;
                    quantity = ""; // Default to empty if no quantity part found
                    std::cout << " (Hint: Ingredient '" << name << "' has no quantity specified. Quantity will be empty)" << std::endl;
                }
                ingredients.push_back({name, quantity});
            }
            return ingredients;
        }

        std::vector<std::string> getStepsFromConsole()
        {
            std::vector<std::string> steps;
            std::cout << "Enter cooking steps (one step per line, type 'done' or empty line to finish):" << std::endl;
            std::string step_str;
            int stepNumber = 1;
            while (true)
            {
                std::cout << "Step " << stepNumber << ": ";
                std::getline(std::cin, step_str);
                if (step_str == "done" || step_str == "DONE" || step_str.empty())
                {
                    if (steps.empty() && step_str.empty())
                    {
                        std::string confirmEnd = getStringFromConsole("No steps entered. Are you sure you want to finish? (y/n): ");
                        if (confirmEnd != "y" && confirmEnd != "Y")
                            continue;
                    }
                    break;
                }
                steps.push_back(step_str);
                stepNumber++;
            }
            return steps;
        }

        void displayRecipeDetailsBrief(const RecipeApp::Recipe &recipe)
        {
            std::cout << "  ID: " << recipe.getRecipeId() << ", Name: " << recipe.getName() << ", Cuisine: " << recipe.getCuisine() << std::endl;
        }

        void displayRecipeDetailsFull(const RecipeApp::Recipe &recipe)
        {
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "Recipe ID: " << recipe.getRecipeId() << std::endl;
            std::cout << "Name: " << recipe.getName() << std::endl;
            std::cout << "Cuisine: " << recipe.getCuisine() << std::endl;
            std::cout << "Cooking Time: " << recipe.getCookingTime() << " minutes" << std::endl;
            std::cout << "Difficulty: ";
            switch (recipe.getDifficulty())
            {
            case RecipeApp::Difficulty::Easy:
                std::cout << "Easy";
                break;
            case RecipeApp::Difficulty::Medium:
                std::cout << "Medium";
                break;
            case RecipeApp::Difficulty::Hard:
                std::cout << "Hard";
                break;
            }
            std::cout << std::endl;

            std::cout << "Ingredients:" << std::endl;
            if (recipe.getIngredients().empty())
            {
                std::cout << "  (No ingredient information)" << std::endl;
            }
            else
            {
                for (const auto &ing : recipe.getIngredients())
                {
                    std::cout << "  - " << ing.first << " (" << ing.second << ")" << std::endl; // Assuming quantity is in ing.second
                }
            }

            std::cout << "Steps:" << std::endl;
            if (recipe.getSteps().empty())
            {
                std::cout << "  (No step information)" << std::endl;
            }
            else
            {
                int stepNum = 1;
                for (const auto &step : recipe.getSteps())
                {
                    std::cout << "  " << stepNum++ << ". " << step << std::endl; // Display step number
                }
            }
            if (recipe.getNutritionalInfo().has_value() && !recipe.getNutritionalInfo().value().empty())
            {
                std::cout << "Nutritional Info: " << recipe.getNutritionalInfo().value() << std::endl;
            }
            if (recipe.getImageUrl().has_value() && !recipe.getImageUrl().value().empty())
            {
                std::cout << "Image URL: " << recipe.getImageUrl().value() << std::endl;
            }
            std::cout << "----------------------------------------" << std::endl;
        }

    } // namespace CliUtils
} // namespace RecipeApp