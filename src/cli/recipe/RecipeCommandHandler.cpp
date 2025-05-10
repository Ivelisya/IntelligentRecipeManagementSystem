#include "RecipeCommandHandler.h"
#include "spdlog/spdlog.h" // For logging
#include "../../common/exceptions/ValidationException.h" // For custom exceptions

#include <iostream> // For std::cout, std::cerr (will be phased out for logging where appropriate)
#include <limits>     // Required for std::numeric_limits
#include <stdexcept>  // Required for std::exception
#include <string>
#include <vector>

#include "../../domain/recipe/Recipe.h"
#include "../CliUtils.h"
#include "../ExitCodes.h"
#include <algorithm>

namespace RecipeApp {
namespace CliHandlers {

// parseCsvStringToVector moved to CliUtils

RecipeCommandHandler::RecipeCommandHandler(RecipeApp::RecipeManager &rm)
    : recipeManager(rm) {}

int RecipeCommandHandler::handleAddRecipe(const cxxopts::ParseResult &result) {
    std::cout << "--- 添加新菜谱 ---" << std::endl;
    std::string name =
        RecipeApp::CliUtils::getStringFromConsole("请输入菜谱名称: ");
    if (name.empty()) {
        spdlog::error("菜谱名称不能为空。");
        throw Common::Exceptions::ValidationException("菜谱名称不能为空。");
    }

    // Name conflict check is now handled by recipeManager.addRecipe and will throw ValidationException
    // std::vector<RecipeApp::Recipe> existingRecipes =
    //     recipeManager.findRecipeByName(name, false);
    // if (!existingRecipes.empty()) {
    //     spdlog::error("菜谱名称 '{}' 已存在，请使用不同的名称。", name);
    //     throw Common::Exceptions::ValidationException("菜谱名称 '" + name + "' 已存在。");
    // }

    std::vector<std::pair<std::string, std::string>> ingredients =
        RecipeApp::CliUtils::getIngredientsFromConsole();
    if (ingredients.empty()) {
        // Consider if ingredients can be empty. For now, let's assume they can't for a new recipe.
        // spdlog::warn("菜谱至少需要一种配料。");
        // throw Common::Exceptions::ValidationException("菜谱至少需要一种配料。");
    }
    std::vector<std::string> steps = RecipeApp::CliUtils::getStepsFromConsole();
    if (steps.empty()) {
        // Consider if steps can be empty.
        // spdlog::warn("菜谱至少需要一个步骤。");
        // throw Common::Exceptions::ValidationException("菜谱至少需要一个步骤。");
    }

    int cookingTime = 0;
    bool validTime = false;
    while(!validTime) {
        cookingTime = RecipeApp::CliUtils::getIntFromConsole(
            "请输入烹饪时长 (分钟, 正整数): ");
        if (cookingTime <= 0) {
            spdlog::error("烹饪时长必须为正整数。");
            // Optionally, re-prompt or throw. For now, re-prompt is handled by loop.
            // To throw: throw Common::Exceptions::ValidationException("烹饪时长必须为正整数。");
        } else {
            validTime = true;
        }
    }
    RecipeApp::Difficulty difficulty =
        RecipeApp::CliUtils::getDifficultyFromConsole();

    // Get Tags (Cuisine is now part of Tags)
    std::vector<std::string> tags = RecipeApp::CliUtils::getTagsFromConsole();

    std::string nutritionalInfo = RecipeApp::CliUtils::getStringFromConsole(
        "请输入营养信息 (可选, 可为空): ");
    std::string imageUrl = RecipeApp::CliUtils::getStringFromConsole(
        "请输入图片链接 (可选, 可为空): ");

    // Note: The Recipe constructor no longer takes 'cuisine' directly.
    // It's assumed to be handled as one of the tags if needed.
    std::vector<RecipeApp::Ingredient> domain_ingredients;
    for (const auto &ing_pair : ingredients) {
        domain_ingredients.push_back({ing_pair.first, ing_pair.second});
    }

    auto builder = RecipeApp::Recipe::builder(0, name)
                       .withIngredients(domain_ingredients)
                       .withSteps(steps)
                       .withCookingTime(cookingTime)
                       .withDifficulty(difficulty)
                       .withTags(tags);
    if (!nutritionalInfo.empty()) {
        builder.withNutritionalInfo(nutritionalInfo);
    }
    if (!imageUrl.empty()) {
        builder.withImageUrl(imageUrl);
    }
    RecipeApp::Recipe newRecipe = builder.build();

    int newRecipeId = recipeManager.addRecipe(newRecipe);

    if (newRecipeId != -1) {
        std::cout << "菜谱 '" << name << "' 添加成功！ (新 ID: " << newRecipeId
                  << ")" << std::endl;
        return RecipeApp::Cli::EX_OK;
    } else {
        // Error already logged by RecipeManager or caught as specific exception
        // spdlog::error("添加菜谱失败。"); // This is too generic here
        return RecipeApp::Cli::EX_APP_OPERATION_FAILED; // Or rely on exceptions from manager
    }
}

int RecipeCommandHandler::handleListRecipes(
    const cxxopts::ParseResult &result) {
    std::cout << "--- 菜谱列表 ---" << std::endl;
    const auto &allRecipes = recipeManager.getAllRecipes();
    if (allRecipes.empty()) {
        std::cout << "当前没有可用的菜谱。" << std::endl;
        return RecipeApp::Cli::EX_OK;  // Not an error, just no data
    }
    for (const auto &recipe : allRecipes) {
        RecipeApp::CliUtils::displayRecipeDetailsBrief(recipe);
    }
    std::cout << "共 " << allRecipes.size() << " 个菜谱。" << std::endl;
    return RecipeApp::Cli::EX_OK;
}

int RecipeCommandHandler::handleViewRecipe(const cxxopts::ParseResult &result) {
    if (!result.count("recipe-view")) {
        spdlog::error("查看菜谱命令 (--recipe-view) 需要一个菜谱ID参数。");
        throw Common::Exceptions::ValidationException("查看菜谱需要提供菜谱ID。");
    }

    int recipeId = 0;
    try {
        recipeId = result["recipe-view"].as<int>();
        if (recipeId <= 0) {
            spdlog::error("提供的菜谱ID '{}' 无效，ID必须为正整数。", recipeId);
            throw Common::Exceptions::ValidationException("无效的菜谱ID: ID必须为正整数。");
        }
    } catch (const cxxopts::exceptions::exception &e) {
        spdlog::error("解析菜谱ID时出错: {}。提供的参数: '{}'。", e.what(), result["recipe-view"].as<std::string>());
        throw Common::Exceptions::ValidationException("无效的菜谱ID格式，请输入一个数字。");
    }

    std::optional<RecipeApp::Recipe> recipeOpt =
        recipeManager.findRecipeById(recipeId);
    if (recipeOpt)
    {
        RecipeApp::CliUtils::displayRecipeDetailsFull(recipeOpt.value());
        return RecipeApp::Cli::EX_OK;
    } else {
        spdlog::warn("未找到ID为 {} 的菜谱。", recipeId);
        // Consider throwing a specific NotFoundException or returning a specific code
        // For now, let main.cpp's catch-all handle or return a specific code
        std::cout << "未找到ID为 " << recipeId << " 的菜谱。" << std::endl; // User-facing message
        return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
    }
}

int RecipeCommandHandler::handleSearchRecipes(
    const cxxopts::ParseResult &result) {
    std::vector<RecipeApp::Recipe> recipesToDisplay;
    std::string searchCriteriaDisplay;
    bool nameQueryProvided = false;
    bool tagQueryProvided = false;

    // 1. Filter by name if --recipe-search <query> is provided
    if (result.count("recipe-search") &&
        !result["recipe-search"].as<std::string>().empty()) {
        std::string nameQuery = result["recipe-search"].as<std::string>();
        // Use the updated RecipeManager method that utilizes indexes
        recipesToDisplay = recipeManager.findRecipeByName(nameQuery, true);
        searchCriteriaDisplay = "名称包含: \"" + nameQuery + "\"";
        nameQueryProvided = true;
    }
    // recipesToDisplay now either contains name-matched recipes or is empty.

    // 2. Filter by tags (--tags takes precedence over --tag)
    std::vector<std::string> tagsToSearch;
    bool matchAllSpecifiedTags = true;  // Default for --tags
    std::string tagCriteriaDisplayPart;

    if (result.count("tags")) {
        std::string csv_tags = result["tags"].as<std::string>();
        tagsToSearch = RecipeApp::CliUtils::parseCsvStringToVector(csv_tags);
        if (!tagsToSearch.empty()) {
            tagCriteriaDisplayPart = "标签组匹配 (全部): \"" + csv_tags + "\"";
            tagQueryProvided = true;
        }
    } else if (result.count("tag")) {
        std::string single_tag_str = result["tag"].as<std::string>();
        if (!single_tag_str.empty()) {
            tagsToSearch.push_back(single_tag_str);
            // For a single --tag, RecipeManager::findRecipesByTags with
            // matchAll=false or RecipeManager::findRecipesByTag can be used.
            // Let's assume findRecipesByTags handles single tag with
            // matchAll=false correctly.
            matchAllSpecifiedTags = false;
            tagCriteriaDisplayPart = "标签包含: \"" + single_tag_str + "\"";
            tagQueryProvided = true;
        }
    }

    if (tagQueryProvided && !tagsToSearch.empty()) {
        if (nameQueryProvided) {  // Both name and tags are provided
            searchCriteriaDisplay += " 并且 " + tagCriteriaDisplayPart;
            if (recipesToDisplay.empty()) {
                // Name search yielded no results, so combined search will also
                // be empty.
            } else {
                // Get IDs from name search results
                std::set<int> name_matched_ids;
                for (const auto &r : recipesToDisplay)
                    name_matched_ids.insert(r.getRecipeId());

                // Get IDs from tag search results
                std::vector<RecipeApp::Recipe> tag_only_results =
                    recipeManager.findRecipesByTags(tagsToSearch,
                                                    matchAllSpecifiedTags);

                std::set<int> tag_matched_ids;
                for (const auto &r : tag_only_results)
                    tag_matched_ids.insert(r.getRecipeId());

                // Intersect the ID sets
                std::set<int> final_ids;
                std::set_intersection(
                    name_matched_ids.begin(), name_matched_ids.end(),
                    tag_matched_ids.begin(), tag_matched_ids.end(),
                    std::inserter(final_ids, final_ids.begin()));

                if (final_ids.empty()) {
                    recipesToDisplay.clear();
                } else {
                    std::vector<int> final_ids_vec(final_ids.begin(),
                                                   final_ids.end());
                    // Fetch the actual recipe objects for the final IDs
                    recipesToDisplay =
                        recipeManager.findRecipesByIds(final_ids_vec);
                }
            }
        } else {  // Only tags are provided (nameQueryProvided is false)
            searchCriteriaDisplay = tagCriteriaDisplayPart;
            recipesToDisplay = recipeManager.findRecipesByTags(
                tagsToSearch, matchAllSpecifiedTags);
        }
    } else if (!nameQueryProvided && !tagQueryProvided) {
        // Neither name nor tag query provided.
    }

    // Check if any search criteria was actually provided
    if (!nameQueryProvided && !tagQueryProvided) {
        // This means --recipe-search was called without a query, and no --tag
        // or --tags. Or just --recipe-search with an empty string. The main.cpp
        // option definition should ideally prevent --recipe-search without any
        // value. If it's allowed, it implies "list all", but that's
        // `recipe-list`. For a "search" command, some criteria should be
        // present.
        if (result.count("recipe-search")) {
            spdlog::error("请为搜索提供查询词或标签。");
            throw Common::Exceptions::ValidationException(
                "请为搜索提供查询词或标签。用法: --recipe-search [查询词] [--tag <标签>] [--tags <标签1,标签2>]");
        }
        // If not even --recipe-search command, this handler shouldn't be
        // called. But if it is, and no criteria, it's an issue. For safety, if
        // searchCriteriaDisplay is empty, list all.
        if (searchCriteriaDisplay.empty())
            searchCriteriaDisplay = "所有菜谱 (无有效过滤器)";
    }

    std::cout << "--- 菜谱搜索结果 (" << searchCriteriaDisplay << ") ---"
              << std::endl;

    if (recipesToDisplay.empty()) {
        std::cout << "未找到匹配的菜谱。" << std::endl;
    } else {
        for (const auto &recipe : recipesToDisplay) {
            RecipeApp::CliUtils::displayRecipeDetailsBrief(recipe);
        }
        std::cout << "找到 " << recipesToDisplay.size() << " 个匹配的菜谱。"
                  << std::endl;
    }
    return RecipeApp::Cli::EX_OK;
}

int RecipeCommandHandler::handleUpdateRecipe(
    const cxxopts::ParseResult &result) {
    if (!result.count("recipe-update")) {
        spdlog::error("更新菜谱命令 (--recipe-update) 需要一个菜谱ID参数。");
        throw Common::Exceptions::ValidationException("更新菜谱需要提供菜谱ID。");
    }

    int recipeId = 0;
    try {
        recipeId = result["recipe-update"].as<int>();
        if (recipeId <= 0) {
            spdlog::error("提供的菜谱ID '{}' 无效，ID必须为正整数。", recipeId);
            throw Common::Exceptions::ValidationException("无效的菜谱ID: ID必须为正整数。");
        }
    } catch (const cxxopts::exceptions::exception &e) {
        spdlog::error("解析菜谱ID时出错: {}。提供的参数: '{}'。", e.what(), result["recipe-update"].as<std::string>());
        throw Common::Exceptions::ValidationException("无效的菜谱ID格式，请输入一个数字。");
    }

    std::optional<RecipeApp::Recipe> recipeToUpdateOpt =
        recipeManager.findRecipeById(recipeId);
    if (!recipeToUpdateOpt) {
        spdlog::warn("尝试更新但未找到ID为 {} 的菜谱。", recipeId);
        std::cout << "错误：未找到ID为 " << recipeId << " 的菜谱。" << std::endl; // User-facing
        return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
    }
    RecipeApp::Recipe recipeToUpdate = recipeToUpdateOpt.value();

    std::cout << "--- 更新菜谱 (ID: " << recipeId << ") ---" << std::endl;
    std::cout << "当前菜谱信息：" << std::endl;
    RecipeApp::CliUtils::displayRecipeDetailsFull(recipeToUpdate);
    std::cout << "请输入新的菜谱信息 (留空则表示保留当前值)：" << std::endl;

    std::string newName = RecipeApp::CliUtils::getStringFromConsole(
        "新名称 [" + recipeToUpdate.getName() + "]: ");
    if (!newName.empty()) {
        // Name conflict check is handled by RecipeManager::updateRecipe.
        // If it returns false due to conflict, the final error message will be generic.
        // For a more specific error here, RecipeManager::updateRecipe would need to throw ValidationException.
        recipeToUpdate.setName(newName);
    }

    std::cout << "修改配料? (y/n, 当前 "
              << recipeToUpdate.getIngredients().size() << " 项): ";
    std::string changeIngredients =
        RecipeApp::CliUtils::getStringFromConsole("");
    if (changeIngredients == "y" || changeIngredients == "Y") {
        std::vector<std::pair<std::string, std::string>> ing_pairs_update =
            RecipeApp::CliUtils::getIngredientsFromConsole();
        std::vector<RecipeApp::Ingredient> domain_ingredients_update;
        for (const auto &ing_pair : ing_pairs_update) {
            domain_ingredients_update.push_back(
                {ing_pair.first, ing_pair.second});
        }
        recipeToUpdate.setIngredients(domain_ingredients_update);
    }

    std::cout << "修改步骤? (y/n, 当前 " << recipeToUpdate.getSteps().size()
              << " 项): ";
    std::string changeSteps = RecipeApp::CliUtils::getStringFromConsole("");
    if (changeSteps == "y" || changeSteps == "Y") {
        recipeToUpdate.setSteps(RecipeApp::CliUtils::getStepsFromConsole());
    }

    // Update Tags
    std::string currentTagsDisplay;
    const auto &currentTags = recipeToUpdate.getTags();
    for (size_t i = 0; i < currentTags.size(); ++i) {
        currentTagsDisplay +=
            currentTags[i] + (i == currentTags.size() - 1 ? "" : ",");
    }
    // Use the new interactive way to get tags for update
    if (result.count("tags")) {
        // If --tags is provided via command line for update, use it directly.
        // This maintains consistency for non-interactive updates.
        std::string new_tags_csv_cmd = result["tags"].as<std::string>();
        recipeToUpdate.setTags(
            RecipeApp::CliUtils::parseCsvStringToVector(new_tags_csv_cmd));
        std::cout << "标签已通过命令行参数更新。" << std::endl;
    } else {
        // Interactive update for tags
        std::cout << "要修改标签吗? (当前: "
                  << (currentTagsDisplay.empty() ? "无" : currentTagsDisplay)
                  << ")" << std::endl;
        // We call getTagsFromConsole, passing current tags.
        // The function itself will ask if user wants to keep/clear/append.
        recipeToUpdate.setTags(
            RecipeApp::CliUtils::getTagsFromConsole(recipeToUpdate.getTags()));
    }

    std::string newTimeStr = RecipeApp::CliUtils::getStringFromConsole(
        "新烹饪时长 (分钟) [" +
        std::to_string(recipeToUpdate.getCookingTime()) + "]: ");
    if (!newTimeStr.empty()) {
        try {
            int newTime = std::stoi(newTimeStr);
            if (newTime > 0)
                recipeToUpdate.setCookingTime(newTime);
            else
                std::cout << "输入的烹饪时长无效。值将保持不变。" << std::endl;
        } catch (const std::exception &e) {
            (void)e;
            std::cout << "输入的烹饪时长不是有效数字。值将保持不变。"
                      << std::endl;
        }
    }

    std::cout << "修改难度? (y/n): ";
    std::string changeDifficulty =
        RecipeApp::CliUtils::getStringFromConsole("");
    if (changeDifficulty == "y" || changeDifficulty == "Y") {
        recipeToUpdate.setDifficulty(
            RecipeApp::CliUtils::getDifficultyFromConsole());
    }

    // Cuisine is now part of tags, so direct update of cuisine is removed.
    // Users should modify tags if they want to change cuisine.

    std::string newNutritionalInfo = RecipeApp::CliUtils::getStringFromConsole(
        "新营养信息 [" + recipeToUpdate.getNutritionalInfo().value_or("") +
        "]: ");
    if (!newNutritionalInfo.empty())
        recipeToUpdate.setNutritionalInfo(newNutritionalInfo);
    else {
        std::string clearInfo =
            RecipeApp::CliUtils::getStringFromConsole("清除营养信息? (y/n): ");
        if (clearInfo == "y" || clearInfo == "Y")
            recipeToUpdate.setNutritionalInfo({});
    }

    std::string newImageUrl = RecipeApp::CliUtils::getStringFromConsole(
        "新图片链接 [" + recipeToUpdate.getImageUrl().value_or("") + "]: ");
    if (!newImageUrl.empty())
        recipeToUpdate.setImageUrl(newImageUrl);
    else {
        std::string clearUrl =
            RecipeApp::CliUtils::getStringFromConsole("清除图片链接? (y/n): ");
        if (clearUrl == "y" || clearUrl == "Y") recipeToUpdate.setImageUrl({});
    }

    if (recipeManager.updateRecipe(recipeToUpdate)) {
        std::cout << "菜谱 ID " << recipeId << " 更新成功！" << std::endl;
        spdlog::info("菜谱 ID {} 更新成功。", recipeId);
        return RecipeApp::Cli::EX_OK;
    } else {
        // RecipeManager::updateRecipe might have logged more specific reasons if it failed (e.g., name conflict if not throwing).
        spdlog::error("更新菜谱 ID {} 失败。", recipeId);
        std::cout << "更新菜谱失败。请检查日志获取更多信息。" << std::endl; // User-facing
        return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
    }
}

int RecipeCommandHandler::handleDeleteRecipe(
    const cxxopts::ParseResult &result) {
    // const RecipeApp::User *currentUser = userManager.getCurrentUser(); // No
    // longer needed for auth check if (!currentUser) // This check is no longer
    // needed
    // {
    //     std::cerr << "Error: Please login first to delete a recipe." <<
    //     std::endl; return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
    // }

    // if (currentUser->getRole() != RecipeApp::UserRole::Admin) // This check
    // is no longer needed
    // {
    //     std::cerr << "Error: Permission denied. Only administrators can
    //     delete recipes." << std::endl; return
    //     RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
    // }

    if (!result.count("recipe-delete")) {
        spdlog::error("删除菜谱命令 (--recipe-delete) 需要一个菜谱ID参数。");
        throw Common::Exceptions::ValidationException("删除菜谱需要提供菜谱ID。");
    }

    int recipeId = 0;
    try {
        recipeId = result["recipe-delete"].as<int>();
        if (recipeId <= 0) {
            spdlog::error("提供的菜谱ID '{}' 无效，ID必须为正整数。", recipeId);
            throw Common::Exceptions::ValidationException("无效的菜谱ID: ID必须为正整数。");
        }
    } catch (const cxxopts::exceptions::exception &e) {
        spdlog::error("解析菜谱ID时出错: {}。提供的参数: '{}'。", e.what(), result["recipe-delete"].as<std::string>());
        throw Common::Exceptions::ValidationException("无效的菜谱ID格式，请输入一个数字。");
    }

    std::optional<RecipeApp::Recipe> recipeToDeleteOpt =
        recipeManager.findRecipeById(recipeId);
    if (!recipeToDeleteOpt) {
        spdlog::warn("尝试删除但未找到ID为 {} 的菜谱。", recipeId);
        std::cout << "错误：未找到ID为 " << recipeId << " 的菜谱。" << std::endl; // User-facing
        return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
    }

    std::cout << "找到菜谱: " << recipeToDeleteOpt.value().getName()
              << " (ID: " << recipeId << ")" << std::endl;
    std::string confirm = RecipeApp::CliUtils::getStringFromConsole(
        "您确定要删除这个菜谱吗？ (y/n): ");

    if (confirm == "y" || confirm == "Y") {
        if (recipeManager.deleteRecipe(recipeId)) {
            std::cout << "菜谱 ID " << recipeId << " 删除成功！" << std::endl;
            return RecipeApp::Cli::EX_OK;
        } else {
            std::cerr << "删除菜谱失败。" << std::endl;
            return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
        }
    } else {
        std::cout << "删除操作已取消。" << std::endl;
        return RecipeApp::Cli::EX_OK;  // User cancelled, not an error
    }
}

}  // namespace CliHandlers
}  // namespace RecipeApp