#define _CRT_SECURE_NO_WARNINGS  // Suppress getenv warnings on MSVC
#include <filesystem>            // Required for std::filesystem
#include <iostream>
#include <limits>     // Required for std::numeric_limits
#include <stdexcept>  // Required for std::exception
#include <string>
#include <vector>

#include "cxxopts.hpp"  // 包含 cxxopts 头文件

// Core Logic Managers
// #include "logic/user/UserManager.h" // UserManager removed
#include "logic/recipe/RecipeManager.h"
#include "logic/restaurant/RestaurantManager.h"  // Keep for now, will need repository later
#include "logic/encyclopedia/RecipeEncyclopediaManager.h" // ADDED for encyclopedia

// Persistence Layer (Repositories)
// #include "persistence/PersistenceManager.h" // No longer needed by
// UserCommandHandler #include "persistence/JsonUserRepository.h"       //
// JsonUserRepository removed
#include "persistence/JsonRecipeRepository.h"      // ADDED
#include "persistence/JsonRestaurantRepository.h"  // ADDED

// Domain Objects (needed for enums, etc.)
#include "domain/recipe/Recipe.h"
// #include "domain/user/User.h" // User domain object removed
// #include "domain/restaurant/Restaurant.h" // Will be needed later

// CLI Utilities and Handlers
#include "cli/CliUtils.h"                     // For CLI utility functions
#include "cli/recipe/RecipeCommandHandler.h"  // Include the new Recipe handler
#include "cli/restaurant/RestaurantCommandHandler.h"
#include "cli/user/UserCommandHandler.h"
#include "cli/encyclopedia/RecipeEncyclopediaCommandHandler.h" // ADDED: New encyclopedia handler
// #include "cli/handlers/AdminCommandHandler.h"  // AdminCommandHandler removed
#include "cli/ExitCodes.h"  // Include ExitCodes

// 版本号可以定义为常量
const std::string APP_VERSION = "0.1.0";

int main(int argc, char *argv[]) {
    // --- Determine User Configuration Directory ---
    std::filesystem::path configDirPath;
#ifdef _WIN32
    const char *appData = std::getenv("APPDATA");
    if (appData) {
        configDirPath = std::filesystem::path(appData) /
                        "IntelligentRecipeManagementSystem";
    } else {
        // Fallback or error handling if APPDATA is not set
        configDirPath =
            std::filesystem::current_path() /
            ".IntelligentRecipeManagementSystem_UserConfig";  // Fallback
        std::cerr << "Warning: APPDATA environment variable not found. Using "
                     "fallback config directory: "
                  << configDirPath << std::endl;
    }
#else  // Assuming Linux/macOS
    const char *homeDir = std::getenv("HOME");
    if (homeDir) {
        configDirPath = std::filesystem::path(homeDir) / ".config" /
                        "IntelligentRecipeManagementSystem";
    } else {
        // Fallback or error handling if HOME is not set
        configDirPath =
            std::filesystem::current_path() /
            ".IntelligentRecipeManagementSystem_UserConfig";  // Fallback
        std::cerr << "Warning: HOME environment variable not found. Using "
                     "fallback config directory: "
                  << configDirPath << std::endl;
    }
#endif

    try {
        if (!std::filesystem::exists(configDirPath)) {
            std::filesystem::create_directories(configDirPath);
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Error: Could not create configuration directory: "
                  << configDirPath << " - " << e.what() << std::endl;
        return RecipeApp::Cli::EX_CANTCREAT;
    }

    // --- Dependency Injection Setup ---

    // 1. Instantiate Repositories using the config directory path
    // RecipeApp::Persistence::JsonUserRepository userRepository("users.json");
    // // JsonUserRepository removed
    RecipeApp::Persistence::JsonRecipeRepository recipeRepository(
        configDirPath, "recipes.json");
    RecipeApp::Persistence::JsonRestaurantRepository restaurantRepository(
        configDirPath, "restaurants.json");  // ADDED Instantiation with path

    // 2. Load data into repositories
    // User data loading removed
    if (!recipeRepository.load()) {
        std::cerr << "错误：无法加载菜谱数据 (recipes.json)。程序将退出。"
                  << std::endl;
        return RecipeApp::Cli::EX_DATAERR;
    }
    if (!restaurantRepository.load()) {  // ADDED Loading
        std::cerr << "错误：无法加载餐厅数据 (restaurants.json)。程序将退出。"
                  << std::endl;
        return RecipeApp::Cli::EX_DATAERR;
    }
    std::cout << "数据加载成功。"
              << std::endl;  // Or move this inside handlers if needed

    // 3. Instantiate Managers with Repository Dependencies
    // RecipeApp::UserManager userManager(userRepository);                   //
    // UserManager removed
    RecipeApp::RecipeManager recipeManager(
        recipeRepository);  // Inject RecipeRepository
    RecipeApp::RestaurantManager restaurantManager(
        restaurantRepository);  // ADDED Injection

    // Instantiate RecipeEncyclopediaManager
    RecipeApp::Logic::Encyclopedia::RecipeEncyclopediaManager encyclopediaManager;

    // Determine the path to encyclopedia_recipes.json relative to the executable
    std::filesystem::path executablePath(argv[0]); // Path to the executable
    std::filesystem::path projectRootPath = executablePath.parent_path().parent_path(); // Assuming executable is in build/Debug or build/Release
    if (executablePath.filename() == "recipe-cli" || executablePath.filename() == "recipe-cli.exe") {
         // If directly in 'build' (e.g. from 'cmake --build build' then 'cd build && Debug\recipe-cli.exe')
         // or if it's in a subdirectory of build like 'build/Debug'
        if (executablePath.parent_path().filename() == "Debug" || executablePath.parent_path().filename() == "Release") {
            projectRootPath = executablePath.parent_path().parent_path().parent_path(); // build/Debug/exe -> build/Debug -> build -> project_root
        } else {
             projectRootPath = executablePath.parent_path().parent_path(); // build/exe -> build -> project_root
        }
    } else {
        // Fallback or if structure is different, assume current_path might be project root
        // This part might need adjustment based on actual deployment/execution scenarios
        projectRootPath = std::filesystem::current_path();
        std::cerr << "Warning: Could not reliably determine project root from executable path. Assuming current_path is project root for encyclopedia data." << std::endl;
    }

    std::filesystem::path encyclopediaDataPathFs = projectRootPath / "data" / "encyclopedia_recipes.json";
    std::string encyclopediaDataPath = encyclopediaDataPathFs.string();

    if (RecipeApp::CliUtils::isVerbose()) {
        std::cout << "[调试] 尝试从以下路径加载食谱大全: " << encyclopediaDataPath << std::endl;
        std::cout << "[调试] 可执行文件路径: " << executablePath.string() << std::endl;
        std::cout << "[调试] 推断的项目根目录: " << projectRootPath.string() << std::endl;
    }

    if (!encyclopediaManager.loadRecipes(encyclopediaDataPath)) {
        std::cerr << "警告：无法加载食谱大全数据 (" << encyclopediaDataPath
                  << ")。食谱大全功能可能不可用。" << std::endl;
    } else {
        if (RecipeApp::CliUtils::isVerbose()) {
             std::cout << "[调试] 食谱大全数据从 " << encyclopediaDataPath << " 加载成功。" << std::endl;
        }
    }

    // 4. Instantiate Command Handlers with Manager Dependencies
    RecipeApp::CliHandlers::RecipeCommandHandler recipeCommandHandler(
        recipeManager);
    RecipeApp::CliHandlers::RestaurantCommandHandler restaurantCommandHandler(
        restaurantManager);
    RecipeApp::CliHandlers::UserCommandHandler userCommandHandler;
    RecipeApp::CliHandlers::RecipeEncyclopediaCommandHandler encyclopediaCommandHandler(encyclopediaManager); // ADDED: Instantiate new handler
    // RecipeApp::CliHandlers::AdminCommandHandler
    // adminCommandHandler(userManager); // AdminCommandHandler removed

    // --- Old PersistenceManager Loading REMOVED ---
    // RecipeApp::PersistenceManager persistenceManager("recipes.json",
    // "users.json", "restaurants.json"); RecipeApp::UserManager
    // userManager_old; RecipeApp::RecipeManager recipeManager_old;
    // RecipeApp::RestaurantManager restaurantManager_old;
    // if (!persistenceManager.loadData(userManager_old, recipeManager_old,
    // restaurantManager_old)) { ... }

    // --- CLI Options Parsing ---
    cxxopts::Options options("recipe-cli",
                             "菜谱命令行工具 - 管理您的菜谱和用户");
    options.set_width(100);

    options.add_options()("h,help", "打印此帮助信息并退出")(
        "v,version", "打印应用程序版本号并退出")(
        "verbose", "启用详细输出模式，显示更多调试信息。");

    // options.add_options("User")("login", "User login.\n  Example: recipe-cli
    // --login myuser\n  Example: recipe-cli --login",
    // cxxopts::value<std::string>()->implicit_value(""),
    // "USERNAME")("register", "Register a new user.\n  Example: recipe-cli
    // --register newuser\n  Example: recipe-cli --register",
    // cxxopts::value<std::string>()->implicit_value(""), "USERNAME")("logout",
    // "Logout the current user.\n  Example: recipe-cli
    // --logout")("user-profile", "Display current logged-in user's details.
    // Requires login.\n  Example: recipe-cli --user-profile")("update-profile",
    // "Update current logged-in user's password. Requires login.\n  Example:
    // recipe-cli --update-profile"); User commands removed. Help text for
    // "User" group might need removal or rephrasing if group is empty.
    options.add_options("Recipe")(
        "recipe-add",
        u8"添加一个新菜谱。\n  可附加 --tags \"标签1,标签2\" 来指定标签。\n  "
        u8"例如: recipe-cli --recipe-add [--tags \"家常菜,快捷\"]")(
        "recipe-list",
        u8"列出所有可用的菜谱。\n  例如: recipe-cli --recipe-list")(
        "recipe-search",
        u8"按名称和/或标签搜索菜谱。\n  名称搜索: --recipe-search \"鸡肉\"\n  "
        u8"单标签搜索: --recipe-search --tag \"素食\"\n  多标签搜索 (AND): "
        u8"--recipe-search --tags \"晚餐,快捷\"\n  组合搜索: --recipe-search "
        u8"\"汤\" --tag \"冬季\"",
        cxxopts::value<std::string>()->implicit_value(""), u8"查询内容 (可选)")(
        "recipe-view",
        u8"按 ID 查看菜谱详情。\n  例如: recipe-cli --recipe-view 101",
        cxxopts::value<int>(),
        u8"菜谱ID")("recipe-update",
                    u8"按 ID 更新菜谱。\n  可附加 --tags \"新标签1,新标签2\" "
                    u8"来更新标签 (将替换所有旧标签)。\n  例如: recipe-cli "
                    u8"--recipe-update 101 [--tags \"健康,午餐\"]",
                    cxxopts::value<int>(), u8"菜谱ID")(
        "recipe-delete",
        u8"按 ID 删除菜谱。\n  例如: recipe-cli --recipe-delete 101",
        cxxopts::value<int>(), u8"菜谱ID")
        // Tag specific options for search and update
        ("tag", u8"用于 --recipe-search, 按单个标签过滤。",
         cxxopts::value<std::string>(), u8"标签名")(
            "tags",
            u8"用于 --recipe-add, --recipe-update (替换) 或 --recipe-search "
            u8"(AND 匹配)。\n  格式: \"标签1,标签2,标签3\"",
            cxxopts::value<std::string>(), u8"逗号分隔的标签列表");

    options.add_options("Encyclopedia")(
        "enc-list", u8"列出食谱大全中的所有菜谱。\n  例如: recipe-cli --enc-list")
        ("enc-search", u8"在食谱大全中按关键词 (名称、食材、标签) 搜索菜谱。\n  例如: recipe-cli encyclopedia search --keywords \"鸡肉 汤\"", // Updated description
         cxxopts::value<std::string>(), u8"搜索关键词 (必需)") // Made keywords mandatory for the handler
        ("enc-view", u8"按 ID 查看食谱大全中特定菜谱的详细信息。\n  例如: recipe-cli encyclopedia view --id 123", // Added new option
         cxxopts::value<int>(), u8"菜谱ID (必需)");

    // options.add_options("Admin")
    // ("admin-user-list", u8"列出系统中的所有用户。\n  例如: recipe-cli
    // --admin-user-list")
    // ("admin-user-create", u8"创建一个新用户。\n  例如: recipe-cli
    // --admin-user-create")
    // ("admin-user-update", u8"按 ID 更新用户信息。\n  例如: recipe-cli
    // --admin-user-update 201", cxxopts::value<int>(), u8"用户ID");
    // ("admin-user-delete", u8"按 ID 删除用户。\n  例如: recipe-cli
    // --admin-user-delete 201", cxxopts::value<int>(), u8"用户ID");

    try {
        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help({"", "Recipe", "Encyclopedia"}) // Added Encyclopedia to help
                      << std::endl;
            return RecipeApp::Cli::EX_OK;
        }
        if (result.count("version")) {
            std::cout << "菜谱命令行工具 版本 " << APP_VERSION << std::endl;
            return RecipeApp::Cli::EX_OK;
        }

        if (result.count("verbose")) {
            RecipeApp::CliUtils::setVerbose(true);
            if (RecipeApp::CliUtils::isVerbose()) {
                std::cout << "[调试] 详细输出模式已启用。" << std::endl;
            }
        }

        // --- Command Dispatching ---
        int exit_code = RecipeApp::Cli::EX_OK;
        bool command_handled = false;

        // User Commands (Removed)
        // if (result.count("login")) { ... }
        // else if (result.count("register")) { ... }
        // else if (result.count("logout")) { ... }
        // else if (result.count("user-profile")) { ... }
        // else if (result.count("update-profile")) { ... }

        // Recipe Commands
        if (result.count("recipe-add"))  // First actual command check now
        {
            exit_code = recipeCommandHandler.handleAddRecipe(result);
            command_handled = true;
        } else if (result.count("recipe-list")) {
            exit_code = recipeCommandHandler.handleListRecipes(result);
            command_handled = true;
        } else if (result.count("recipe-view")) {
            exit_code = recipeCommandHandler.handleViewRecipe(result);
            command_handled = true;
        } else if (result.count("recipe-search")) {
            exit_code = recipeCommandHandler.handleSearchRecipes(result);
            command_handled = true;
        } else if (result.count("recipe-update")) {
            exit_code = recipeCommandHandler.handleUpdateRecipe(result);
            command_handled = true;
        } else if (result.count("recipe-delete")) {
            exit_code = recipeCommandHandler.handleDeleteRecipe(result);
            command_handled = true;
        }
        // Encyclopedia Commands
        else if (result.count("enc-list")) {
            command_handled = true;
            const auto& allEncyclopediaRecipes = encyclopediaManager.getAllRecipes();
            if (allEncyclopediaRecipes.empty()) {
                std::cout << "食谱大全中当前没有菜谱。" << std::endl;
            } else {
                std::cout << "--- 食谱大全 ---" << std::endl;
                for (const auto& recipe : allEncyclopediaRecipes) {
                    // Basic display, can be enhanced using CliUtils if a suitable function exists
                    std::cout << "  ID: " << recipe.getId() << ", 名称: " << recipe.getName();
                    if (!recipe.getTags().empty()) {
                        std::cout << ", 标签: ";
                        for (size_t i = 0; i < recipe.getTags().size(); ++i) {
                            std::cout << recipe.getTags()[i] << (i == recipe.getTags().size() - 1 ? "" : ", ");
                        }
                    }
                    std::cout << std::endl;
                }
                std::cout << "共 " << allEncyclopediaRecipes.size() << " 个菜谱。" << std::endl;
            }
            exit_code = RecipeApp::Cli::EX_OK;
        } else if (result.count("enc-search")) {
            // Delegate to the new handler
            exit_code = encyclopediaCommandHandler.handleSearchEncyclopediaRecipes(result);
            command_handled = true;
        } else if (result.count("enc-view")) {
            // Delegate to the new handler
            exit_code = encyclopediaCommandHandler.handleViewEncyclopediaRecipe(result);
            command_handled = true;
        }
        // Admin Commands (Commented out)
        // else if (result.count("admin-user-list"))
        // {
        //     exit_code = adminCommandHandler.handleAdminUserList(result);
        //     command_handled = true;
        // }
        // else if (result.count("admin-user-create"))
        // {
        //     exit_code = adminCommandHandler.handleAdminUserCreate(result);
        //     command_handled = true;
        // }
        // else if (result.count("admin-user-update")) // Temporarily uncomment
        // for testing
        // {
        //     exit_code = adminCommandHandler.handleAdminUserUpdate(result);
        //     command_handled = true;
        // }
        // else if (result.count("admin-user-delete"))
        // {
        //     exit_code = adminCommandHandler.handleAdminUserDelete(result);
        //     command_handled = true;
        // }
        // Default / No Command
        else if (argc == 1) {
            std::cout << "欢迎使用菜谱命令行工具！" << std::endl;
            std::cout << "使用 'recipe-cli --help' 查看可用命令。" << std::endl;
            command_handled = true;
        } else  // Arguments given, but no known command matched
        {
            if (!command_handled) {  // Check again if truly unhandled
                bool only_global_options_without_command = true;
                // Check if any actual command option was present
                for (const auto &cmd_opt : { // Added new encyclopedia commands to this check
                         "recipe-add", "recipe-list", "recipe-search",
                         "recipe-view", "recipe-update", "recipe-delete",
                         "enc-list", "enc-search", "enc-view" // Added enc-view to check
                         // "admin-user-update" // Temporarily add back for
                         // testing "admin-user-list", "admin-user-create",
                         // "admin-user-delete" // Commented out
                     }) {
                    if (result.count(cmd_opt)) {
                        only_global_options_without_command = false;
                        break;
                    }
                }

                if (only_global_options_without_command &&
                    (result.count("verbose"))) {
                    // Only global flags like --verbose passed without a
                    // command. Not an error.
                    std::cout << "使用 'recipe-cli --help' 查看可用命令。"
                              << std::endl;
                    command_handled = true;  // Consider it handled.
                } else if (!only_global_options_without_command) {
                    // A command flag was likely present but didn't match the
                    // dispatch logic (shouldn't happen ideally)
                    std::cerr << "错误：无法识别的命令。请检查命令拼写。"
                              << std::endl;
                    exit_code = RecipeApp::Cli::EX_USAGE;
                    command_handled = true;
                } else {
                    // Arguments were present but didn't match any known option
                    // or command
                    std::cerr
                        << "错误：无效参数。使用 'recipe-cli --help' 获取帮助。"
                        << std::endl;
                    exit_code = RecipeApp::Cli::EX_USAGE;
                    command_handled = true;
                }
            }
        }

        if (RecipeApp::CliUtils::isVerbose() && command_handled) {
            std::cout << "[调试] 命令已处理，退出码: " << exit_code
                      << std::endl;
        }
        return exit_code;
    } catch (const cxxopts::exceptions::exception &e) {
        std::cerr << "错误：解析命令行参数失败: " << e.what() << std::endl;
        std::cerr << "使用 'recipe-cli --help' 获取帮助。" << std::endl;
        return RecipeApp::Cli::EX_USAGE;
    } catch (const std::exception &e) {
        std::cerr << "发生意外的内部错误: " << e.what() << std::endl;
        if (RecipeApp::CliUtils::isVerbose()) {
            std::cerr << "[调试] 异常类型: " << typeid(e).name() << std::endl;
        }
        return RecipeApp::Cli::EX_SOFTWARE;
    }
    // Fallback
    return RecipeApp::Cli::EX_SOFTWARE;
}