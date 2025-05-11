#define _CRT_SECURE_NO_WARNINGS  // Suppress getenv warnings on MSVC
#include <filesystem>            // Required for std::filesystem
#include <iostream>
#include <limits>     // Required for std::numeric_limits
#include <stdexcept>  // Required for std::exception
#include <string>
#include <vector>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
// #include "spdlog/sinks/basic_file_sink.h" // For potential file logging later

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
// #include "cli/user/UserCommandHandler.h" // Removed as part of P1.7
#include "cli/encyclopedia/RecipeEncyclopediaCommandHandler.h" // ADDED: New encyclopedia handler
// #include "cli/handlers/AdminCommandHandler.h"  // AdminCommandHandler removed
#include "cli/ExitCodes.h"  // Include ExitCodes
#include "common/exceptions/ValidationException.h"
#include "common/exceptions/PersistenceException.h"
#include "common/exceptions/BusinessLogicException.h"
#include "common/exceptions/ConfigurationException.h"
// BaseException.h is transitively included

// 版本号可以定义为常量
const std::string APP_VERSION = "3.2.0";

int main(int argc, char *argv[]) {
    // --- Early basic logging setup (before full config, in case of early errors) ---
    // This can be a very simple console logger initially.
    // We will set up a more sophisticated one after config path is determined.
    auto preliminary_console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    preliminary_console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    auto preliminary_logger = std::make_shared<spdlog::logger>("prelim_console", preliminary_console_sink);
    spdlog::set_default_logger(preliminary_logger);
    spdlog::set_level(spdlog::level::info); // Default to info for pre-config phase

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
        spdlog::warn("APPDATA environment variable not found. Using fallback config directory: {}", configDirPath.string());
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
        spdlog::warn("HOME environment variable not found. Using fallback config directory: {}", configDirPath.string());
    }
#endif

    try {
        if (!std::filesystem::exists(configDirPath)) {
            if (std::filesystem::create_directories(configDirPath)) {
                // Use spdlog if available, otherwise fallback for this specific early message
                if (spdlog::default_logger()) {
                    spdlog::info("Configuration directory created: {}", configDirPath.string());
                } else {
                    std::cout << "Info: Configuration directory created: " << configDirPath.string() << std::endl;
                }
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        // Use spdlog if available, otherwise fallback for this specific early message
        if (spdlog::default_logger()) {
            spdlog::error("Could not create configuration directory: {} - {}", configDirPath.string(), e.what());
        } else {
            std::cerr << "Error: Could not create configuration directory: "
                      << configDirPath.string() << " - " << e.what() << std::endl;
        }
        return RecipeApp::Cli::EX_CANTCREAT;
    }

    // --- Initialize Main Logging System ---
    // Now that configDirPath is known, we could set up file logging if desired.
    // For now, we'll stick to a console logger but re-initialize to ensure it's the primary.
    try {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v"); // Standard pattern
        // Example for file sink (can be enabled later via config or verbose flag)
        // std::filesystem::path logFilePath = configDirPath / "recipe-cli.log";
        // auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
        // file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v");

        // std::vector<spdlog::sink_ptr> sinks{console_sink}; //, file_sink};
        // auto main_logger = std::make_shared<spdlog::logger>("recipe_cli_logger", sinks.begin(), sinks.end());
        auto main_logger = std::make_shared<spdlog::logger>("console", console_sink);

        spdlog::set_default_logger(main_logger);
        spdlog::set_level(spdlog::level::info); // Default level, overridden by --verbose later
        spdlog::flush_on(spdlog::level::info); // Flush on info and above

        spdlog::info("Main logging system initialized. Config directory: {}", configDirPath.string());

    } catch (const spdlog::spdlog_ex& ex) {
        // Fallback to cerr if logger setup fails
        // This std::cerr is acceptable here as it's a fallback for logger failure itself.
        std::cerr << "Main log system initialization failed: " << ex.what() << std::endl;
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
        spdlog::error("无法加载菜谱数据 (recipes.json)。程序将退出。");
        return RecipeApp::Cli::EX_DATAERR;
    }
    spdlog::info("菜谱数据 (recipes.json) 加载成功。");

    if (!restaurantRepository.load()) {  // ADDED Loading
        spdlog::error("无法加载餐厅数据 (restaurants.json)。程序将退出。");
        return RecipeApp::Cli::EX_DATAERR;
    }
    spdlog::info("餐厅数据 (restaurants.json) 加载成功。");

    // 3. Instantiate Managers with Repository Dependencies
    // RecipeApp::UserManager userManager(userRepository);                   //
    // UserManager removed
    RecipeApp::RecipeManager recipeManager(
        recipeRepository);  // Inject RecipeRepository
    RecipeApp::RestaurantManager restaurantManager(
        restaurantRepository);  // ADDED Injection

    // Instantiate RecipeEncyclopediaManager
    RecipeApp::Logic::Encyclopedia::RecipeEncyclopediaManager encyclopediaManager;

    // --- Determine the path to encyclopedia_recipes.json (Improved Logic P1.5) ---
    std::string encyclopediaDataPath;
    std::filesystem::path execPath;
    try {
        execPath = std::filesystem::absolute(std::filesystem::path(argv[0]));
    } catch (const std::exception& e) {
        spdlog::error("无法获取可执行文件的绝对路径: {}", e.what());
        // Fallback or decide how to handle, for now, let it proceed to try other paths
        execPath = std::filesystem::path(argv[0]); // Use as is
    }
    std::filesystem::path execDir = execPath.parent_path();

    std::vector<std::filesystem::path> potentialPaths;
    // 1. Relative to executable: ./data/encyclopedia_recipes.json (e.g. deployed alongside data dir)
    potentialPaths.push_back(execDir / "data" / "encyclopedia_recipes.json");
    // 2. Relative to executable: ../data/encyclopedia_recipes.json (common for build/config/exe)
    potentialPaths.push_back(execDir / ".." / "data" / "encyclopedia_recipes.json");
    // 3. Relative to executable: ../../data/encyclopedia_recipes.json (common for build/config/sub/exe)
    potentialPaths.push_back(execDir / ".." / ".." / "data" / "encyclopedia_recipes.json");
    // 4. Project root relative to common build structures (e.g. build/Debug/exe -> project_root/data)
    if (execDir.has_parent_path() && execDir.parent_path().has_parent_path()) { // execDir/../.. (project_root)
        potentialPaths.push_back(execDir.parent_path().parent_path() / "data" / "encyclopedia_recipes.json");
        if (execDir.parent_path().parent_path().has_parent_path()){ // execDir/../../.. (e.g. if build is in a sub-sub-dir)
             potentialPaths.push_back(execDir.parent_path().parent_path().parent_path() / "data" / "encyclopedia_recipes.json");
        }
    }
    // 5. In user config directory (less common for encyclopedia, but a fallback)
    potentialPaths.push_back(configDirPath / "encyclopedia_recipes.json");
    // 6. In current working directory: ./data/encyclopedia_recipes.json
    potentialPaths.push_back(std::filesystem::current_path() / "data" / "encyclopedia_recipes.json");
    // 7. Directly in current working directory (if file is just encyclopedia_recipes.json)
    potentialPaths.push_back(std::filesystem::current_path() / "encyclopedia_recipes.json");
     // 8. Directly relative to executable (if file is just encyclopedia_recipes.json)
    potentialPaths.push_back(execDir / "encyclopedia_recipes.json");


    spdlog::debug("开始查找食谱大全数据文件 (encyclopedia_recipes.json)。可执行文件路径: {}", execPath.string());
    for (const auto& p : potentialPaths) {
        std::filesystem::path canonical_path;
        try {
            // weakly_canonical to resolve ., .. without requiring the file to exist initially for all parts of the path
            canonical_path = std::filesystem::weakly_canonical(p);
        } catch (const std::filesystem::filesystem_error& fs_err) {
            spdlog::debug("  - 检查路径时发生错误 (路径: '{}'): {}", p.string(), fs_err.what());
            canonical_path = p; // Use original path if canonicalization fails
        }
        spdlog::debug("  - 正在检查规范化路径: {}", canonical_path.string());
        if (std::filesystem::exists(canonical_path) && std::filesystem::is_regular_file(canonical_path)) {
            encyclopediaDataPath = canonical_path.string();
            spdlog::info("食谱大全数据文件找到于: {}", encyclopediaDataPath);
            break;
        }
    }

    if (encyclopediaDataPath.empty()) {
        spdlog::warn("无法在任何预期位置找到食谱大全数据文件 (encyclopedia_recipes.json)。食谱大全功能可能不可用。");
        // encyclopediaManager.loadRecipes will handle an empty path if necessary
    }

    if (!encyclopediaManager.loadRecipes(encyclopediaDataPath)) { // loadRecipes should handle empty path gracefully
        spdlog::warn("无法加载食谱大全数据 ({}). 食谱大全功能可能不可用。", encyclopediaDataPath);
    } else {
        if (RecipeApp::CliUtils::isVerbose()) { // This check is fine
             spdlog::debug("食谱大全数据从 {} 加载成功。", encyclopediaDataPath);
        }
    }

    // 4. Instantiate Command Handlers with Manager Dependencies
    RecipeApp::CliHandlers::RecipeCommandHandler recipeCommandHandler(
        recipeManager);
    RecipeApp::CliHandlers::RestaurantCommandHandler restaurantCommandHandler(
        restaurantManager, recipeManager); // Added recipeManager
    // RecipeApp::CliHandlers::UserCommandHandler userCommandHandler; // Removed as part of P1.7
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
                             "菜谱命令行工具 - 管理您的菜谱、餐馆和浏览食谱大全"); // 更新描述
    options.set_width(100);

    options.add_options()("h,help", "打印此帮助信息并退出")(
        "v,version", "打印应用程序版本号并退出")(
        "verbose", "启用详细输出模式，显示更多调试信息。");

    // User commands removed.
    options.add_options("Recipe")(
        "recipe-add",
        u8"添加一个新菜谱 (交互式)。\n"
        u8"  可选参数 --tags \"标签1,标签2\" 可用于在添加时预设标签。\n"
        u8"  例如 1: recipe-cli --recipe-add\n"
        u8"  例如 2: recipe-cli --recipe-add --tags \"家常菜,快捷\"",
        // The implicit value for recipe-add is not directly used for a name,
        // as adding is interactive. It's kept for cxxopts structure.
        cxxopts::value<std::string>()->implicit_value("unused_placeholder_for_add"), u8"菜谱名称 (通过交互式输入)")(
        "recipe-list",
        u8"列出所有已保存的菜谱。\n  例如: recipe-cli --recipe-list")(
        "recipe-search",
        u8"按名称和/或标签搜索已保存的菜谱。\n"
        u8"  用法 1 (按名称): recipe-cli --recipe-search \"宫保鸡丁\"\n"
        u8"  用法 2 (按单个标签): recipe-cli --recipe-search --tag \"川菜\"\n"
        u8"  用法 3 (按多个独立标签 AND 匹配): recipe-cli --recipe-search --tag \"晚餐\" --tag \"快捷\"\n"
        u8"  用法 4 (按逗号分隔的多标签 AND 匹配): recipe-cli --recipe-search --tags \"晚餐,快捷\"\n"
        u8"  用法 5 (名称和单标签): recipe-cli --recipe-search \"汤\" --tag \"冬季\"\n"
        u8"  用法 6 (名称和多标签): recipe-cli --recipe-search \"沙拉\" --tags \"夏季,健康\"\n"
        u8"  注意: 如果只提供标签，则仅按标签搜索。如果同时提供名称和标签，则进行组合搜索。",
        cxxopts::value<std::string>()->implicit_value(""), u8"搜索关键词 (可选)")(
        "recipe-view",
        u8"按 ID 查看已保存菜谱的详细信息。\n  例如: recipe-cli --recipe-view 101",
        cxxopts::value<int>(),
        u8"菜谱ID (必需)")("recipe-update",
                    u8"按 ID 更新已保存的菜谱 (交互式)。\n"
                    u8"  可选参数 --tags \"新标签1,新标签2\" 可用于更新标签 (将替换所有旧标签)。\n"
                    u8"  例如 1: recipe-cli --recipe-update 101\n"
                    u8"  例如 2: recipe-cli --recipe-update 101 --tags \"健康,午餐\"",
                    cxxopts::value<int>(), u8"菜谱ID (必需)")(
        "recipe-delete",
        u8"按 ID 删除已保存的菜谱。\n  例如: recipe-cli --recipe-delete 101",
        cxxopts::value<int>(), u8"菜谱ID (必需)")
        ("tag", u8"用于 --recipe-search，按单个标签过滤。可多次使用以进行 AND 匹配 (例如 --tag \"素食\" --tag \"快捷\")。",
         cxxopts::value<std::vector<std::string>>(), u8"标签名")( // Changed to vector for multiple --tag
            "tags",
            u8"用于 --recipe-add (预设标签), --recipe-update (替换所有标签), 或 --recipe-search (AND 匹配所有列出的标签)。\n"
            u8"  格式为逗号分隔的字符串: \"标签1,标签2,标签3\"\n"
            u8"  例如 (搜索): recipe-cli --recipe-search --tags \"晚餐,快捷\"",
            cxxopts::value<std::string>(), u8"逗号分隔的标签列表");

    options.add_options("Encyclopedia")(
        "enc-list", u8"列出食谱大全中的所有菜谱条目。\n  例如: recipe-cli --enc-list")
        ("enc-search", u8"在食谱大全中按关键词 (如名称、食材、标签) 搜索菜谱。\n  例如: recipe-cli --enc-search \"美味的鸡肉汤\"",
         cxxopts::value<std::string>(), u8"搜索关键词 (必需)")
        ("enc-view", u8"按 ID 查看食谱大全中特定菜谱的详细信息。\n  例如: recipe-cli --enc-view 123",
         cxxopts::value<int>(), u8"菜谱ID (必需)");

    options.add_options("Restaurant")(
        "restaurant-add",
        u8"添加一个新餐馆 (交互式)。\n  例如: recipe-cli --restaurant-add")(
        "restaurant-list",
        u8"列出所有已保存的餐馆。\n  例如: recipe-cli --restaurant-list")(
        "restaurant-view",
        u8"按 ID 查看已保存餐馆的详情。\n  例如: recipe-cli --restaurant-view 1",
        cxxopts::value<int>(), u8"餐馆ID (必需)")(
        "restaurant-update",
        u8"按 ID 更新已保存餐馆的信息 (交互式)。\n  例如: recipe-cli --restaurant-update 1",
        cxxopts::value<int>(), u8"餐馆ID (必需)")(
        "restaurant-delete",
        u8"按 ID 删除已保存的餐馆。\n  例如: recipe-cli --restaurant-delete 1",
        cxxopts::value<int>(), u8"餐馆ID (必需)");
    // Admin commands removed

    try {
        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            // 显示所有定义的命令组
            std::cout << options.help({"", "Recipe", "Encyclopedia", "Restaurant"})
                      << std::endl;
            return RecipeApp::Cli::EX_OK;
        }
        if (result.count("version")) {
            std::cout << "菜谱命令行工具 版本 " << APP_VERSION << std::endl;
            return RecipeApp::Cli::EX_OK;
        }

        if (result.count("verbose")) {
            RecipeApp::CliUtils::setVerbose(true);
            spdlog::set_level(spdlog::level::debug); // Set spdlog level to debug
            spdlog::debug("Verbose output enabled via command line.");
            // The old std::cout for this is no longer needed.
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
        // Restaurant Commands
        else if (result.count("restaurant-add")) {
            exit_code = restaurantCommandHandler.handleAddRestaurant(result);
            command_handled = true;
        } else if (result.count("restaurant-list")) {
            exit_code = restaurantCommandHandler.handleListRestaurants(result);
            command_handled = true;
        } else if (result.count("restaurant-view")) {
            exit_code = restaurantCommandHandler.handleViewRestaurant(result);
            command_handled = true;
        } else if (result.count("restaurant-update")) {
            exit_code = restaurantCommandHandler.handleUpdateRestaurant(result);
            command_handled = true;
        } else if (result.count("restaurant-delete")) {
            exit_code = restaurantCommandHandler.handleDeleteRestaurant(result);
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
                         "enc-list", "enc-search", "enc-view", // Added enc-view to check
                         "restaurant-add", "restaurant-list", "restaurant-view", "restaurant-update", "restaurant-delete" // Added restaurant commands
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
                    spdlog::error("无法识别的命令。请检查命令拼写。");
                    exit_code = RecipeApp::Cli::EX_USAGE;
                    command_handled = true;
                } else {
                    // Arguments were present but didn't match any known option
                    // or command
                    spdlog::error("无效参数。使用 'recipe-cli --help' 获取帮助。");
                    exit_code = RecipeApp::Cli::EX_USAGE;
                    command_handled = true;
                }
            }
        }

        if (RecipeApp::CliUtils::isVerbose() && command_handled) {
            spdlog::debug("命令已处理，退出码: {}", exit_code);
        }
        return exit_code;
    } catch (const cxxopts::exceptions::exception &e) {
        spdlog::error("解析命令行参数失败: {}", e.what());
        spdlog::info("使用 'recipe-cli --help' 获取帮助。");
        return RecipeApp::Cli::EX_USAGE;
    } catch (const RecipeApp::Common::Exceptions::ValidationException &e) {
        spdlog::error("输入校验失败: {}", e.what());
        spdlog::info("请检查您的输入并重试。使用 '--help' 获取命令用法。");
        return RecipeApp::Cli::EX_USAGE;
    } catch (const RecipeApp::Common::Exceptions::PersistenceException &e) {
        spdlog::error("数据持久化错误: {}", e.what());
        spdlog::info("请检查文件权限或数据文件是否损坏。");
        return RecipeApp::Cli::EX_DATAERR;
    } catch (const RecipeApp::Common::Exceptions::BusinessLogicException &e) {
        spdlog::error("业务逻辑错误: {}", e.what());
        spdlog::info("操作无法完成。");
        return RecipeApp::Cli::EX_SOFTWARE;
    } catch (const RecipeApp::Common::Exceptions::ConfigurationException &e) {
        spdlog::error("配置错误: {}", e.what());
        spdlog::info("请检查应用程序配置。");
        return RecipeApp::Cli::EX_CONFIG;
    } catch (const RecipeApp::Common::Exceptions::RecipeCliBaseException &e) {
        // Catch-all for any other custom exceptions deriving from our base
        spdlog::error("应用程序特定错误: {}", e.what());
        return RecipeApp::Cli::EX_SOFTWARE;
    } catch (const std::exception &e) {
        spdlog::critical("发生未预料的内部标准库错误: {}", e.what());
        if (RecipeApp::CliUtils::isVerbose()) {
            spdlog::debug("异常类型: {}", typeid(e).name());
        }
        return RecipeApp::Cli::EX_SOFTWARE;
    }
    // Fallback
    return RecipeApp::Cli::EX_SOFTWARE;
}