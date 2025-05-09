#include <iostream>   // 用于调试输出
#include <stdexcept>  // 用于异常处理
#include <string>
#include <vector>

#include "../../include/json.hpp"
#include "../domain/recipe/Recipe.h"
#include "../logic/recipe/RecipeManager.h"

// --- DLL 导出宏定义 ---
#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

// --- 使用命名空间 ---
using namespace RecipeApp;
using json = nlohmann::json;

// --- DLL 内部静态实例 ---
static RecipeManager *global_recipe_manager_ptr = nullptr;
static Domain::Recipe::RecipeRepository *global_recipe_repository_ptr =
    nullptr;  // 使用接口类型
// 如果确定只用Json实现，也可以是 JsonRecipeRepository*
// static Persistence::JsonRecipeRepository* global_json_recipe_repository_ptr =
// nullptr;

// --- 导出函数实现 ---

extern "C" {

/**
 * @brief (可选) 初始化菜谱系统。
 *        例如，可以在这里从文件加载数据到 RecipeManager。
 *        Python 端应在开始使用其他 API 前调用此函数（如果需要）。
 */
DLL_EXPORT void initialize_recipe_system() {
    try {
        if (global_recipe_manager_ptr == nullptr) {
            // 决定仓库类型和路径
            // 为了简单起见，这里硬编码使用 JsonRecipeRepository 和 "data" 目录
            // 在实际应用中，这些路径可能来自配置文件或环境变量
            std::filesystem::path baseDir =
                "data";  // DLL 运行时的工作目录下的 data 文件夹
            // 确保目录存在，如果JsonRecipeRepository的构造函数不处理，这里需要处理
            if (!std::filesystem::exists(baseDir)) {
                std::filesystem::create_directories(baseDir);
            }

            // 显式使用 Persistence 命名空间或完全限定名
            auto concrete_repo =
                new RecipeApp::Persistence::JsonRecipeRepository(
                    baseDir, "recipes.json");
            global_recipe_repository_ptr = concrete_repo;

            if (concrete_repo->load()) {  // JsonRecipeRepository 有 load 方法
                std::cout << "[DLL] Recipe data loaded for repository."
                          << std::endl;
            } else {
                std::cout << "[DLL] Failed to load recipe data for repository, "
                             "or starting fresh."
                          << std::endl;
            }
            // 从仓库加载后，设置 RecipeManager 的 nextId
            // global_recipe_manager_ptr->setNextRecipeIdFromPersistence(concrete_repo->getNextId());
            // // RecipeManager 没有这个方法了，ID管理在仓库

            global_recipe_manager_ptr =
                new RecipeManager(*global_recipe_repository_ptr);
            std::cout << "[DLL] RecipeManager initialized." << std::endl;
        } else {
            std::cout << "[DLL] RecipeManager already initialized."
                      << std::endl;
        }

        std::cout << "[DLL] initialize_recipe_system called. Adding test data "
                     "if manager is valid..."
                  << std::endl;

        if (global_recipe_manager_ptr) {
            // --- 添加测试数据 ---
            try {
                Recipe testRecipe1(
                    0, "测试菜谱1 - 麻婆豆腐",
                    {{"豆腐", "1块"}, {"牛肉末", "50g"}, {"豆瓣酱", "1勺"}},
                    {"步骤1", "步骤2"}, 15, Difficulty::Medium, "川菜");
                testRecipe1.setNutritionalInfo("一些营养信息");

                Recipe testRecipe2(
                    0, "测试菜谱2 - 可乐鸡翅",
                    {{"鸡翅中", "8个"}, {"可乐", "1罐"}, {"姜", "3片"}},
                    {"鸡翅焯水", "放入可乐姜片焖煮", "大火收汁"}, 30,
                    Difficulty::Easy, "家常菜");

                if (global_recipe_manager_ptr->addRecipe(testRecipe1) != -1) {
                    std::cout << "[DLL] Added test recipe 1." << std::endl;
                } else {
                    std::cout << "[DLL] Failed to add test recipe 1 (maybe "
                                 "already exists?)"
                              << std::endl;
                }
                if (global_recipe_manager_ptr->addRecipe(testRecipe2) != -1) {
                    std::cout << "[DLL] Added test recipe 2." << std::endl;
                } else {
                    std::cout << "[DLL] Failed to add test recipe 2 (maybe "
                                 "already exists?)"
                              << std::endl;
                }
            } catch (const std::exception &recipe_ex) {
                std::cerr << "[DLL] Exception while adding test recipes: "
                          << recipe_ex.what() << std::endl;
            }
            // --------------------
        } else {
            std::cerr
                << "[DLL] RecipeManager not initialized. Cannot add test data."
                << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "[DLL] Exception during initialization: " << e.what()
                  << std::endl;
        // 可能需要更复杂的错误处理机制通知调用者
    } catch (...) {
        std::cerr << "[DLL] Unknown exception during initialization."
                  << std::endl;
    }
}

/**
 * @brief (可选) 关闭菜谱系统。
 *        例如，可以在这里将数据保存到文件。
 *        Python 端应在应用程序退出前调用此函数（如果需要）。
 */
DLL_EXPORT void shutdown_recipe_system() {
    try {
        // 示例: 尝试保存数据 (如果仓库有 saveAll 方法)
        // if (global_json_recipe_repository_ptr)
        // global_json_recipe_repository_ptr->saveAll(); 或者通过 RecipeManager
        // (如果它提供了保存所有数据的接口)
        std::cout << "[DLL] shutdown_recipe_system called." << std::endl;

        delete global_recipe_manager_ptr;
        global_recipe_manager_ptr = nullptr;

        // 如果 global_recipe_repository_ptr 是 new 出来的 JsonRecipeRepository*
        // 那么也需要 delete 它
        delete static_cast<RecipeApp::Persistence::JsonRecipeRepository *>(
            global_recipe_repository_ptr);
        global_recipe_repository_ptr = nullptr;

        std::cout << "[DLL] RecipeManager and Repository shut down."
                  << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "[DLL] Exception during shutdown: " << e.what()
                  << std::endl;
    } catch (...) {
        std::cerr << "[DLL] Unknown exception during shutdown." << std::endl;
    }
}

/**
 * @brief 获取所有菜谱数据的 JSON 字符串表示。
 *        此函数在堆上分配内存来存储 JSON 字符串。
 * @return char* 指向包含 JSON 数组字符串的内存。
 *         如果成功，调用者必须稍后使用 free_allocated_string() 释放此内存。
 *         如果发生错误，可能返回指向包含错误信息的 JSON
 * 字符串的指针（也需要释放），或返回 nullptr。
 */
DLL_EXPORT char *get_all_recipes_json_alloc() {
    std::cout << "[DLL DEBUG] Entered get_all_recipes_json_alloc."
              << std::endl;  // DEBUG
    try {
        if (!global_recipe_manager_ptr) {
            std::string error_msg =
                "[DLL] Error: RecipeManager not initialized in "
                "get_all_recipes_json_alloc.";
            std::cerr << error_msg << std::endl;
            json error_json_obj = {{"error", error_msg}};
            std::string error_json_s = error_json_obj.dump();
            char *error_buffer = new char[error_json_s.length() + 1];
            strcpy(error_buffer, error_json_s.c_str());
            return error_buffer;
        }
        std::cout << "[DLL DEBUG] Accessing global recipe manager..."
                  << std::endl;  // DEBUG
        const auto &recipes_list =
            global_recipe_manager_ptr
                ->getAllRecipes();  // 返回 std::vector<Recipe>
        std::cout << "[DLL DEBUG] Got recipe list reference (std::vector)."
                  << std::endl;  // DEBUG
        // 注释已更新: recipes_list 是 std::vector
        // std::cout << "[DLL DEBUG] Recipe list size: " << recipes_list.size()
        // << std::endl; // DEBUG

        json json_array = json::array();
        std::cout
            << "[DLL DEBUG] Starting recipe list iteration (std::vector)..."
            << std::endl;  // DEBUG

        // 遍历 std::vector 并序列化每个 Recipe
        // 范围 for 对 std::vector 同样有效
        int recipe_count = 0;
        for (const auto &recipe : recipes_list) {
            std::cout << "[DLL DEBUG] Processing recipe ID: "
                      << recipe.getRecipeId() << std::endl;  // DEBUG
            json_array.push_back(recipe);  // 调用 RecipeApp::to_json (通过 ADL)
            recipe_count++;
        }
        std::cout << "[DLL DEBUG] Finished iteration (range for). Count: "
                  << recipe_count << std::endl;  // DEBUG

        // 旧的 CustomLinkedList 方案2 注释可以保留或删除，因为它不再相关
        // ...

        // 将 JSON 数组转换为字符串
        std::cout << "[DLL DEBUG] Dumping JSON array to string..."
                  << std::endl;                  // DEBUG
        std::string json_s = json_array.dump();  // dump() 对于空数组会返回 "[]"
        std::cout << "[DLL DEBUG] JSON string length: " << json_s.length()
                  << std::endl;  // DEBUG
        // std::cout << "[DLL DEBUG] JSON string content (first 100 chars): " <<
        // json_s.substr(0, 100) << std::endl; // DEBUG - 打印部分内容

        // 分配内存并复制字符串内容
        std::cout << "[DLL DEBUG] Allocating memory for JSON string..."
                  << std::endl;  // DEBUG
        char *result_buffer = new char[json_s.length() + 1];
        std::cout << "[DLL DEBUG] Memory allocated at: "
                  << static_cast<void *>(result_buffer) << std::endl;  // DEBUG
#ifdef _MSC_VER  // 使用 strcpy_s (如果可用)
        strcpy_s(result_buffer, json_s.length() + 1, json_s.c_str());
#else
        strcpy(result_buffer, json_s.c_str());
#endif
        std::cout << "[DLL DEBUG] JSON string copied to buffer."
                  << std::endl;  // DEBUG

        std::cout
            << "[DLL] get_all_recipes_json_alloc returning JSON data pointer: "
            << static_cast<void *>(result_buffer) << std::endl;  // 调试
        return result_buffer;
    } catch (const json::exception &je) {  // 捕获 JSON 相关的异常
        std::string error_msg =
            "[DLL] JSON Exception in get_all_recipes_json_alloc: " +
            std::string(je.what());
        std::cerr << error_msg << std::endl;
        json error_json_obj = {{"error", error_msg}};
        std::string error_json_s = error_json_obj.dump();
        char *error_buffer = new char[error_json_s.length() + 1];
        strcpy(error_buffer, error_json_s.c_str());  // Added strcpy
        return error_buffer;
    } catch (const std::exception &e) {  // 捕获其他标准异常
        std::string error_msg =
            "[DLL] Standard Exception in get_all_recipes_json_alloc: " +
            std::string(e.what());
        std::cerr << error_msg << std::endl;
        json error_json_obj = {{"error", error_msg}};
        std::string error_json_s = error_json_obj.dump();
        char *error_buffer = new char[error_json_s.length() + 1];
        strcpy(error_buffer, error_json_s.c_str());  // Added strcpy
        return error_buffer;
    } catch (...) {  // 捕获未知异常
        std::cerr << "[DLL] Unknown exception in get_all_recipes_json_alloc"
                  << std::endl;
        json error_json_obj = {
            {"error",
             "Unknown error in DLL during get_all_recipes_json_alloc"}};
        std::string error_json_s = error_json_obj.dump();
        char *error_buffer = new char[error_json_s.length() + 1];
        strcpy(error_buffer, error_json_s.c_str());  // Added strcpy
        return error_buffer;
    }
}

/**
 * @brief 释放由 get_all_recipes_json_alloc() 分配的内存。
 * @param str_ptr 由 get_all_recipes_json_alloc() 返回的指针。
 */
DLL_EXPORT void free_allocated_string(char *str_ptr) {
    std::cout << "[DLL DEBUG] Entered free_allocated_string with pointer: "
              << static_cast<void *>(str_ptr) << std::endl;  // DEBUG
    if (str_ptr != nullptr) {
        std::cout << "[DLL DEBUG] Deleting memory..." << std::endl;  // DEBUG
        delete[] str_ptr;  // 必须与 new char[] 对应
        std::cout << "[DLL DEBUG] Memory deleted." << std::endl;  // DEBUG
    } else {
        std::cout << "[DLL DEBUG] free_allocated_string called with nullptr."
                  << std::endl;  // DEBUG
    }
}

/**
 * @brief 添加新菜谱。
 *        此函数接收包含新菜谱数据的 JSON 字符串，在 RecipeManager 中添加菜谱。
 *        返回操作结果的 JSON 字符串。
 * @param recipe_json_str 包含新菜谱数据的 JSON 字符串。
 * @return char* 指向包含操作结果的 JSON 字符串的指针。
 *         调用者必须使用 free_allocated_string() 释放此内存。
 */
DLL_EXPORT char *add_recipe_json(const char *recipe_json_str) {
    std::cout << "[DLL DEBUG] Entered add_recipe_json." << std::endl;
    try {
        if (!recipe_json_str) {
            std::string error_msg =
                "[DLL] Error: Null JSON string passed to add_recipe_json.";
            std::cerr << error_msg << std::endl;
            json error_json_obj = {{"success", false}, {"error", error_msg}};
            std::string error_json_s = error_json_obj.dump();
            char *error_buffer = new char[error_json_s.length() + 1];
#ifdef _MSC_VER
            strcpy_s(error_buffer, error_json_s.length() + 1,
                     error_json_s.c_str());
#else
            strcpy(error_buffer, error_json_s.c_str());
#endif
            std::cout << "[DLL DEBUG] Returning error JSON for null input: "
                      << static_cast<void *>(error_buffer) << std::endl;
            return error_buffer;
        }

        std::string json_str(recipe_json_str);
        std::cout << "[DLL DEBUG] Received JSON string: " << json_str
                  << std::endl;

        json recipe_json = json::parse(
            json_str, nullptr, false);   // Allow exceptions to be turned off
        if (recipe_json.is_discarded())  // Check if parsing failed
        {
            std::string error_msg =
                "[DLL] Error: Invalid JSON format in add_recipe_json.";
            std::cerr << error_msg << std::endl;
            json error_json_obj = {{"success", false}, {"error", error_msg}};
            std::string error_json_s = error_json_obj.dump();
            char *error_buffer = new char[error_json_s.length() + 1];
#ifdef _MSC_VER
            strcpy_s(error_buffer, error_json_s.length() + 1,
                     error_json_s.c_str());
#else
            strcpy(error_buffer, error_json_s.c_str());
#endif
            std::cout << "[DLL DEBUG] Returning error JSON for invalid JSON: "
                      << static_cast<void *>(error_buffer) << std::endl;
            return error_buffer;
        }

        // Create a Recipe object from JSON
        // Note: ID will be ignored or reassigned by RecipeManager
        // Recipe new_recipe(0, "", {}, {}, 0, Difficulty::Easy, ""); // Default
        // construction might not be ideal It's better if from_json can
        // construct or if Recipe has a default constructor For now, assuming
        // from_json populates a default-constructed or builder-constructed
        // Recipe. The Recipe::from_json in Recipe.h uses a builder, so this
        // should be fine.
        Recipe new_recipe =
            recipe_json.get<Recipe>();  // Let nlohmann/json handle construction
                                        // via from_json

        // Add the recipe using RecipeManager
        if (!global_recipe_manager_ptr) {
            std::string error_msg =
                "[DLL] Error: RecipeManager not initialized in "
                "add_recipe_json.";
            std::cerr << error_msg << std::endl;
            json error_json_obj = {{"success", false}, {"error", error_msg}};
            std::string error_json_s = error_json_obj.dump();
            char *error_buffer = new char[error_json_s.length() + 1];
            strcpy(error_buffer, error_json_s.c_str());
            return error_buffer;
        }
        int new_id = global_recipe_manager_ptr->addRecipe(new_recipe);
        bool added = (new_id != -1);

        json result_json = {{"success", added}};
        if (added) {
            result_json["id"] = new_id;
        } else {
            result_json["error"] =
                "Failed to add recipe. It may already exist or data is "
                "invalid.";
        }

        std::string result_json_s = result_json.dump();
        char *result_buffer = new char[result_json_s.length() + 1];
#ifdef _MSC_VER
        strcpy_s(result_buffer, result_json_s.length() + 1,
                 result_json_s.c_str());
#else
        strcpy(result_buffer, result_json_s.c_str());
#endif
        std::cout << "[DLL DEBUG] Returning result JSON for add_recipe_json: "
                  << static_cast<void *>(result_buffer) << std::endl;
        return result_buffer;
    } catch (
        const json::parse_error &pe)  // More specific catch for parsing errors
    {
        std::string error_msg = "[DLL] JSON Parse Error in add_recipe_json: " +
                                std::string(pe.what());
        std::cerr << error_msg << std::endl;
        json error_json_obj = {{"success", false}, {"error", error_msg}};
        std::string error_json_s = error_json_obj.dump();
        char *error_buffer = new char[error_json_s.length() + 1];
        strcpy(error_buffer,
               error_json_s.c_str());  // Use strcpy consistently for now
        std::cout << "[DLL DEBUG] Returning error JSON for JSON parse error: "
                  << static_cast<void *>(error_buffer) << std::endl;
        return error_buffer;
    } catch (
        const json::exception &je)  // Catch other nlohmann::json exceptions
    {
        std::string error_msg = "[DLL] JSON Exception in add_recipe_json: " +
                                std::string(je.what());
        std::cerr << error_msg << std::endl;
        json error_json_obj = {{"success", false}, {"error", error_msg}};
        std::string error_json_s = error_json_obj.dump();
        char *error_buffer = new char[error_json_s.length() + 1];
        strcpy(error_buffer,
               error_json_s.c_str());  // Use strcpy consistently for now
        std::cout << "[DLL DEBUG] Returning error JSON for JSON exception: "
                  << static_cast<void *>(error_buffer) << std::endl;
        return error_buffer;
    } catch (const std::exception &e) {
        std::string error_msg =
            "[DLL] Standard Exception in add_recipe_json: " +
            std::string(e.what());
        std::cerr << error_msg << std::endl;
        json error_json_obj = {{"success", false}, {"error", error_msg}};
        std::string error_json_s = error_json_obj.dump();
        char *error_buffer = new char[error_json_s.length() + 1];
        strcpy(error_buffer,
               error_json_s.c_str());  // Use strcpy consistently for now
        std::cout << "[DLL DEBUG] Returning error JSON for standard exception: "
                  << static_cast<void *>(error_buffer) << std::endl;
        return error_buffer;
    } catch (...) {
        std::cerr << "[DLL] Unknown exception in add_recipe_json" << std::endl;
        json error_json_obj = {
            {"success", false},
            {"error", "Unknown error in DLL during add_recipe_json"}};
        std::string error_json_s = error_json_obj.dump();
        char *error_buffer = new char[error_json_s.length() + 1];
        strcpy(error_buffer,
               error_json_s.c_str());  // Use strcpy consistently for now
        std::cout << "[DLL DEBUG] Returning error JSON for unknown exception: "
                  << static_cast<void *>(error_buffer) << std::endl;
        return error_buffer;
    }
}

/**
 * @brief 根据ID查找菜谱。
 * @param recipe_id 要查找的菜谱ID。
 * @return char* 指向包含菜谱数据或错误信息的JSON字符串的指针。
 *         调用者必须使用 free_allocated_string() 释放此内存。
 */
DLL_EXPORT char *get_recipe_by_id_json(int recipe_id) {
    std::cout << "[DLL DEBUG] Entered get_recipe_by_id_json with ID: "
              << recipe_id << std::endl;
    try {
        if (!global_recipe_manager_ptr) {
            std::string error_msg =
                "[DLL] Error: RecipeManager not initialized in "
                "get_recipe_by_id_json.";
            std::cerr << error_msg << std::endl;
            json error_json_obj = {{"success", false}, {"error", error_msg}};
            std::string error_json_s = error_json_obj.dump();
            char *error_buffer = new char[error_json_s.length() + 1];
            strcpy(error_buffer, error_json_s.c_str());
            return error_buffer;
        }
        std::optional<Recipe> recipe_opt =
            global_recipe_manager_ptr->findRecipeById(recipe_id);

        if (recipe_opt.has_value()) {
            json recipe_json =
                recipe_opt.value();  // 使用 to_json 转换 (通过 ADL)
            std::string result_json_s = recipe_json.dump();
            char *result_buffer = new char[result_json_s.length() + 1];
#ifdef _MSC_VER
            strcpy_s(result_buffer, result_json_s.length() + 1,
                     result_json_s.c_str());
#else
            strcpy(result_buffer, result_json_s.c_str());
#endif
            std::cout << "[DLL DEBUG] Returning recipe JSON: "
                      << static_cast<void *>(result_buffer) << std::endl;
            return result_buffer;
        } else {
            json error_json = {{"success", false},
                               {"error", "Recipe not found"}};
            std::string error_json_s = error_json.dump();
            char *error_buffer = new char[error_json_s.length() + 1];
#ifdef _MSC_VER
            strcpy_s(error_buffer, error_json_s.length() + 1,
                     error_json_s.c_str());
#else
            strcpy(error_buffer, error_json_s.c_str());
#endif
            std::cout << "[DLL DEBUG] Returning 'not found' JSON: "
                      << static_cast<void *>(error_buffer) << std::endl;
            return error_buffer;
        }
    } catch (const json::exception &je) {
        std::string error_msg =
            "[DLL] JSON Exception in get_recipe_by_id_json: " +
            std::string(je.what());
        std::cerr << error_msg << std::endl;
        json error_json_obj = {{"success", false}, {"error", error_msg}};
        std::string error_json_s = error_json_obj.dump();
        char *error_buffer = new char[error_json_s.length() + 1];
        strcpy(error_buffer,
               error_json_s.c_str());  // Use strcpy consistently for now
        return error_buffer;
    } catch (const std::exception &e) {
        std::string error_msg =
            "[DLL] Standard Exception in get_recipe_by_id_json: " +
            std::string(e.what());
        std::cerr << error_msg << std::endl;
        json error_json_obj = {{"success", false}, {"error", error_msg}};
        std::string error_json_s = error_json_obj.dump();
        char *error_buffer = new char[error_json_s.length() + 1];
        strcpy(error_buffer,
               error_json_s.c_str());  // Use strcpy consistently for now
        return error_buffer;
    } catch (...) {
        std::cerr << "[DLL] Unknown exception in get_recipe_by_id_json"
                  << std::endl;
        json error_json_obj = {
            {"success", false},
            {"error", "Unknown error in DLL during get_recipe_by_id_json"}};
        std::string error_json_s = error_json_obj.dump();
        char *error_buffer = new char[error_json_s.length() + 1];
        strcpy(error_buffer,
               error_json_s.c_str());  // Use strcpy consistently for now
        return error_buffer;
    }
}

}  // extern "C"