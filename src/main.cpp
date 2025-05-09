#include <iostream>
#include <string>
#include <vector>
#include <limits>      // Required for std::numeric_limits
#include <stdexcept>   // Required for std::exception
#include "cxxopts.hpp" // 包含 cxxopts 头文件

// Core Logic Managers
#include "logic/user/UserManager.h"
#include "logic/recipe/RecipeManager.h"
#include "logic/restaurant/RestaurantManager.h" // Keep for now, will need repository later

// Persistence Layer (Repositories)
#include "persistence/PersistenceManager.h"       // TEMPORARILY RE-ADD for UserCommandHandler dependency
#include "persistence/JsonUserRepository.h"       // ADDED
#include "persistence/JsonRecipeRepository.h"     // ADDED
#include "persistence/JsonRestaurantRepository.h" // ADDED

// Domain Objects (needed for enums, etc.)
#include "domain/recipe/Recipe.h"
#include "domain/user/User.h"
// #include "domain/restaurant/Restaurant.h" // Will be needed later

// CLI Utilities and Handlers
#include "core/CustomLinkedList.h"             // For CustomLinkedList type (if still used by handlers)
#include "cli/CliUtils.h"                      // For CLI utility functions
#include "cli/handlers/UserCommandHandler.h"   // Include the new handler
#include "cli/handlers/RecipeCommandHandler.h" // Include the new Recipe handler
#include "cli/handlers/AdminCommandHandler.h"  // Include the new Admin handler
#include "cli/ExitCodes.h"                     // Include ExitCodes

// 版本号可以定义为常量
const std::string APP_VERSION = "0.1.0";

int main(int argc, char *argv[])
{
    // --- Dependency Injection Setup ---

    // 1. Instantiate Repositories
    RecipeApp::Persistence::JsonUserRepository userRepository("users.json");
    RecipeApp::Persistence::JsonRecipeRepository recipeRepository("recipes.json");
    RecipeApp::Persistence::JsonRestaurantRepository restaurantRepository("restaurants.json"); // ADDED Instantiation

    // 2. Load data into repositories
    if (!userRepository.load())
    {
        std::cerr << "Error: Could not load user data (users.json). Program will exit." << std::endl;
        return RecipeApp::Cli::EX_DATAERR;
    }
    if (!recipeRepository.load())
    {
        std::cerr << "Error: Could not load recipe data (recipes.json). Program will exit." << std::endl;
        return RecipeApp::Cli::EX_DATAERR;
    }
    if (!restaurantRepository.load())
    { // ADDED Loading
        std::cerr << "Error: Could not load restaurant data (restaurants.json). Program will exit." << std::endl;
        return RecipeApp::Cli::EX_DATAERR;
    }
    std::cout << "Data loaded successfully." << std::endl; // Or move this inside handlers if needed

    // 3. Instantiate Managers with Repository Dependencies
    RecipeApp::UserManager userManager(userRepository);                   // Inject UserRepository
    RecipeApp::RecipeManager recipeManager(recipeRepository);             // Inject RecipeRepository
    RecipeApp::RestaurantManager restaurantManager(restaurantRepository); // ADDED Injection

    // TEMPORARY: Instantiate PersistenceManager for UserCommandHandler dependency
    RecipeApp::PersistenceManager tempPersistenceManager("recipes.json", "users.json", "restaurants.json");
    // Note: We are NOT calling loadData() on tempPersistenceManager. Repositories handle loading.

    // 4. Instantiate Command Handlers with Manager Dependencies
    // Pass the temporary persistenceManager to UserCommandHandler for now.
    RecipeApp::CliHandlers::UserCommandHandler userCommandHandler(userManager);
    RecipeApp::CliHandlers::RecipeCommandHandler recipeCommandHandler(recipeManager, userManager);
    RecipeApp::CliHandlers::AdminCommandHandler adminCommandHandler(userManager);

    // --- Old PersistenceManager Loading REMOVED ---
    // RecipeApp::PersistenceManager persistenceManager("recipes.json", "users.json", "restaurants.json");
    // RecipeApp::UserManager userManager_old;
    // RecipeApp::RecipeManager recipeManager_old;
    // RecipeApp::RestaurantManager restaurantManager_old;
    // if (!persistenceManager.loadData(userManager_old, recipeManager_old, restaurantManager_old)) { ... }

    // --- CLI Options Parsing ---
    cxxopts::Options options("recipe-cli", "Recipe App CLI - Manage your recipes and users");
    options.set_width(100);

    options.add_options()("h,help", "Print this help message and exit")("v,version", "Print application version and exit")("verbose", "Enable verbose output mode, shows more debug info.");

    options.add_options("User")("login", "User login.\n  Example: recipe-cli --login myuser\n  Example: recipe-cli --login", cxxopts::value<std::string>()->implicit_value(""), "USERNAME")("register", "Register a new user.\n  Example: recipe-cli --register newuser\n  Example: recipe-cli --register", cxxopts::value<std::string>()->implicit_value(""), "USERNAME")("logout", "Logout the current user.\n  Example: recipe-cli --logout")("user-profile", "Display current logged-in user's details. Requires login.\n  Example: recipe-cli --user-profile")("update-profile", "Update current logged-in user's password. Requires login.\n  Example: recipe-cli --update-profile");

    options.add_options("Recipe (requires login)")("recipe-add", "Add a new recipe. Requires login.\n  Example: recipe-cli --recipe-add")("recipe-list", "List all available recipes.\n  Example: recipe-cli --recipe-list")("recipe-search", "Search recipes by name.\n  Example: recipe-cli --recipe-search \"Chicken\"", cxxopts::value<std::string>(), "QUERY")("recipe-view", "View details of a recipe by ID.\n  Example: recipe-cli --recipe-view 101", cxxopts::value<int>(), "RECIPE_ID")("recipe-update", "Update a recipe by ID. Requires login.\n  Example: recipe-cli --recipe-update 101", cxxopts::value<int>(), "RECIPE_ID") // Removed admin requirement for now
        ("recipe-delete", "Delete a recipe by ID. Requires login.\n  Example: recipe-cli --recipe-delete 101", cxxopts::value<int>(), "RECIPE_ID");                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          // Removed admin requirement for now

    options.add_options("Admin (requires admin login)")("admin-user-list", "List all users in the system. Requires admin rights.\n  Example: recipe-cli --admin-user-list")("admin-user-create", "Create a new user. Requires admin rights.\n  Example: recipe-cli --admin-user-create")("admin-user-update", "Update user information by ID. Requires admin rights.\n  Example: recipe-cli --admin-user-update 201", cxxopts::value<int>(), "USER_ID")("admin-user-delete", "Delete a user by ID. Requires admin rights.\n  Example: recipe-cli --admin-user-delete 201", cxxopts::value<int>(), "USER_ID");

    try
    {
        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help({"", "User", "Recipe (requires login)", "Admin (requires admin login)"}) << std::endl;
            return RecipeApp::Cli::EX_OK;
        }
        if (result.count("version"))
        {
            std::cout << "Recipe App CLI Version " << APP_VERSION << std::endl;
            return RecipeApp::Cli::EX_OK;
        }

        if (result.count("verbose"))
        {
            RecipeApp::CliUtils::setVerbose(true);
            if (RecipeApp::CliUtils::isVerbose())
            {
                std::cout << "[Debug] Verbose output mode enabled." << std::endl;
            }
        }

        // --- Command Dispatching ---
        int exit_code = RecipeApp::Cli::EX_OK;
        bool command_handled = false;

        // User Commands
        if (result.count("login"))
        {
            exit_code = userCommandHandler.handleLogin(result);
            command_handled = true;
        }
        else if (result.count("register"))
        {
            exit_code = userCommandHandler.handleRegister(result);
            command_handled = true;
        }
        else if (result.count("logout"))
        {
            exit_code = userCommandHandler.handleLogout(result);
            command_handled = true;
        }
        else if (result.count("user-profile"))
        {
            exit_code = userCommandHandler.handleUserProfile(result);
            command_handled = true;
        }
        else if (result.count("update-profile"))
        {
            exit_code = userCommandHandler.handleUpdateProfile(result);
            command_handled = true;
        }
        // Recipe Commands
        else if (result.count("recipe-add"))
        {
            exit_code = recipeCommandHandler.handleAddRecipe(result);
            command_handled = true;
        }
        else if (result.count("recipe-list"))
        {
            exit_code = recipeCommandHandler.handleListRecipes(result);
            command_handled = true;
        }
        else if (result.count("recipe-view"))
        {
            exit_code = recipeCommandHandler.handleViewRecipe(result);
            command_handled = true;
        }
        else if (result.count("recipe-search"))
        {
            exit_code = recipeCommandHandler.handleSearchRecipes(result);
            command_handled = true;
        }
        else if (result.count("recipe-update"))
        {
            exit_code = recipeCommandHandler.handleUpdateRecipe(result);
            command_handled = true;
        }
        else if (result.count("recipe-delete"))
        {
            exit_code = recipeCommandHandler.handleDeleteRecipe(result);
            command_handled = true;
        }
        // Admin Commands
        else if (result.count("admin-user-list"))
        {
            exit_code = adminCommandHandler.handleAdminUserList(result);
            command_handled = true;
        }
        else if (result.count("admin-user-create"))
        {
            exit_code = adminCommandHandler.handleAdminUserCreate(result);
            command_handled = true;
        }
        else if (result.count("admin-user-update"))
        {
            exit_code = adminCommandHandler.handleAdminUserUpdate(result);
            command_handled = true;
        }
        else if (result.count("admin-user-delete"))
        {
            exit_code = adminCommandHandler.handleAdminUserDelete(result);
            command_handled = true;
        }
        // Default / No Command
        else if (argc == 1)
        {
            std::cout << "Welcome to Recipe App CLI!" << std::endl;
            std::cout << "Use 'recipe-cli --help' to see available commands." << std::endl;
            command_handled = true;
        }
        else // Arguments given, but no known command matched
        {
            if (!command_handled)
            { // Check again if truly unhandled
                bool only_global_options_without_command = true;
                // Check if any actual command option was present
                for (const auto &cmd_opt : {"login", "register", "logout", "user-profile", "update-profile",
                                            "recipe-add", "recipe-list", "recipe-search", "recipe-view", "recipe-update", "recipe-delete",
                                            "admin-user-list", "admin-user-create", "admin-user-update", "admin-user-delete"})
                {
                    if (result.count(cmd_opt))
                    {
                        only_global_options_without_command = false;
                        break;
                    }
                }

                if (only_global_options_without_command && (result.count("verbose")))
                {
                    // Only global flags like --verbose passed without a command. Not an error.
                    std::cout << "Use 'recipe-cli --help' to see available commands." << std::endl;
                    command_handled = true; // Consider it handled.
                }
                else if (!only_global_options_without_command)
                {
                    // A command flag was likely present but didn't match the dispatch logic (shouldn't happen ideally)
                    std::cerr << "Error: Unrecognized command. Please check command spelling." << std::endl;
                    exit_code = RecipeApp::Cli::EX_USAGE;
                    command_handled = true;
                }
                else
                {
                    // Arguments were present but didn't match any known option or command
                    std::cerr << "Error: Invalid arguments. Use 'recipe-cli --help' for assistance." << std::endl;
                    exit_code = RecipeApp::Cli::EX_USAGE;
                    command_handled = true;
                }
            }
        }

        if (RecipeApp::CliUtils::isVerbose() && command_handled)
        {
            std::cout << "[Debug] Command processed, exit code: " << exit_code << std::endl;
        }
        return exit_code;
    }
    catch (const cxxopts::exceptions::exception &e)
    {
        std::cerr << "Error: Failed to parse command line arguments: " << e.what() << std::endl;
        std::cerr << "Use 'recipe-cli --help' for assistance." << std::endl;
        return RecipeApp::Cli::EX_USAGE;
    }
    catch (const std::exception &e)
    {
        std::cerr << "An unexpected internal error occurred: " << e.what() << std::endl;
        if (RecipeApp::CliUtils::isVerbose())
        {
            std::cerr << "[Debug] Exception type: " << typeid(e).name() << std::endl;
        }
        return RecipeApp::Cli::EX_SOFTWARE;
    }
    // Fallback
    return RecipeApp::Cli::EX_SOFTWARE;
}