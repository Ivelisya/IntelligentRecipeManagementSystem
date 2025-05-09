#include "RecipeCommandHandler.h"       // Changed to relative path
#include "../CliUtils.h"                // Changed to relative path
#include "../ExitCodes.h"               // Changed to relative path
#include "../../domain/recipe/Recipe.h" // Changed to relative path
// #include "domain/user/User.h"     // User domain object removed
// #include "core/CustomLinkedList.h" // No longer needed
#include <iostream>
#include <string>
#include <vector>
#include <limits>    // Required for std::numeric_limits
#include <stdexcept> // Required for std::exception
// #include <sstream>   // No longer needed here, moved to CliUtils.cpp
#include <algorithm> // Required for std::all_of, std::find (used in search)

namespace RecipeApp
{
    namespace CliHandlers
    {

        // parseCsvStringToVector moved to CliUtils

        RecipeCommandHandler::RecipeCommandHandler(RecipeApp::RecipeManager &rm)
            : recipeManager(rm) {}

        int RecipeCommandHandler::handleAddRecipe(const cxxopts::ParseResult &result)
        {
            std::cout << "--- 添加新菜谱 ---" << std::endl;
            std::string name = RecipeApp::CliUtils::getStringFromConsole("请输入菜谱名称: ");
            if (name.empty())
            {
                std::cerr << "错误：菜谱名称不能为空。" << std::endl;
                return RecipeApp::Cli::EX_APP_INVALID_INPUT;
            }

            std::vector<RecipeApp::Recipe> existingRecipes = recipeManager.findRecipeByName(name, false);
            if (!existingRecipes.empty())
            {
                std::cerr << "错误：菜谱名称 '" << name << "' 已存在，请使用不同的名称。" << std::endl;
                return RecipeApp::Cli::EX_APP_ALREADY_EXISTS;
            }

            std::vector<std::pair<std::string, std::string>> ingredients = RecipeApp::CliUtils::getIngredientsFromConsole();
            std::vector<std::string> steps = RecipeApp::CliUtils::getStepsFromConsole();
            int cookingTime = RecipeApp::CliUtils::getIntFromConsole("请输入烹饪时长 (分钟, 正整数): ");
            while (cookingTime <= 0)
            {
                std::cerr << "错误：烹饪时长必须为正整数。" << std::endl;
                cookingTime = RecipeApp::CliUtils::getIntFromConsole("请重新输入烹饪时长 (分钟, 正整数): ");
            }
            RecipeApp::Difficulty difficulty = RecipeApp::CliUtils::getDifficultyFromConsole();
            std::string cuisine = RecipeApp::CliUtils::getStringFromConsole("请输入菜系: ");
            if (cuisine.empty())
            {
                std::cerr << "错误：菜系不能为空。" << std::endl;
                return RecipeApp::Cli::EX_APP_INVALID_INPUT;
            }

            // Get Tags
            std::string tags_csv;
            if (result.count("tags"))
            { // Check if --tags option was provided via command line
                tags_csv = result["tags"].as<std::string>();
            }
            else
            { // Otherwise, prompt from console
                tags_csv = RecipeApp::CliUtils::getStringFromConsole("请输入食谱标签 (多个用逗号分隔, 可选): ");
            }
            std::vector<std::string> tags = RecipeApp::CliUtils::parseCsvStringToVector(tags_csv);

            std::string nutritionalInfo = RecipeApp::CliUtils::getStringFromConsole("请输入营养信息 (可选, 可为空): ");
            std::string imageUrl = RecipeApp::CliUtils::getStringFromConsole("请输入图片链接 (可选, 可为空): ");

            RecipeApp::Recipe newRecipe(0, name, ingredients, steps, cookingTime, difficulty, cuisine, tags);
            if (!nutritionalInfo.empty())
                newRecipe.setNutritionalInfo(nutritionalInfo);
            if (!imageUrl.empty())
                newRecipe.setImageUrl(imageUrl);

            int newRecipeId = recipeManager.addRecipe(newRecipe);

            if (newRecipeId != -1)
            {
                std::cout << "菜谱 '" << name << "' 添加成功！ (新 ID: " << newRecipeId << ")" << std::endl;
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "添加菜谱失败。名称可能冲突或发生内部错误。" << std::endl;
                return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
            }
        }

        int RecipeCommandHandler::handleListRecipes(const cxxopts::ParseResult &result)
        {
            std::cout << "--- 菜谱列表 ---" << std::endl;
            const auto &allRecipes = recipeManager.getAllRecipes();
            if (allRecipes.empty())
            {
                std::cout << "当前没有可用的菜谱。" << std::endl;
                return RecipeApp::Cli::EX_OK; // Not an error, just no data
            }
            for (const auto &recipe : allRecipes)
            {
                RecipeApp::CliUtils::displayRecipeDetailsBrief(recipe);
            }
            std::cout << "共 " << allRecipes.size() << " 个菜谱。" << std::endl;
            return RecipeApp::Cli::EX_OK;
        }

        int RecipeCommandHandler::handleViewRecipe(const cxxopts::ParseResult &result)
        {
            if (!result.count("recipe-view"))
            {
                std::cerr << "错误：recipe-view 命令缺少参数 (RECIPE_ID)。" << std::endl;
                std::cerr << "用法: recipe-cli --recipe-view <菜谱ID>" << std::endl;
                return RecipeApp::Cli::EX_USAGE;
            }

            int recipeId = 0;
            try
            {
                recipeId = result["recipe-view"].as<int>();
            }
            catch (const cxxopts::exceptions::exception &e)
            {
                (void)e; // Mark as intentionally unused
                std::cerr << "错误：无效的菜谱ID '" << result["recipe-view"].as<std::string>() << "'。请输入一个数字。" << std::endl;
                return RecipeApp::Cli::EX_DATAERR;
            }

            std::optional<RecipeApp::Recipe> recipeOpt = recipeManager.findRecipeById(recipeId);
            if (recipeOpt) // Or recipeOpt.has_value()
            {
                RecipeApp::CliUtils::displayRecipeDetailsFull(recipeOpt.value());
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "未找到ID为 " << recipeId << " 的菜谱。" << std::endl;
                return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
            }
        }

        int RecipeCommandHandler::handleSearchRecipes(const cxxopts::ParseResult &result)
        {
            std::vector<RecipeApp::Recipe> recipesToDisplay;
            std::string searchCriteriaDisplay;
            bool nameQueryProvided = false;
            bool tagQueryProvided = false;

            // 1. Filter by name if --recipe-search <query> is provided
            if (result.count("recipe-search") && !result["recipe-search"].as<std::string>().empty())
            {
                std::string nameQuery = result["recipe-search"].as<std::string>();
                recipesToDisplay = recipeManager.findRecipeByName(nameQuery, true);
                searchCriteriaDisplay = "名称包含: \"" + nameQuery + "\"";
                nameQueryProvided = true;
            }
            else
            {
                // If no name query, start with all recipes for tag filtering
                recipesToDisplay = recipeManager.getAllRecipes();
            }

            // 2. Filter by tags (--tags takes precedence over --tag)
            std::vector<std::string> tagsToFilterBy;
            bool matchAllTags = true; // Default for --tags is usually AND

            if (result.count("tags"))
            {
                std::string csv_tags = result["tags"].as<std::string>();
                tagsToFilterBy = RecipeApp::CliUtils::parseCsvStringToVector(csv_tags);
                if (!tagsToFilterBy.empty())
                {
                    if (nameQueryProvided)
                        searchCriteriaDisplay += " 并且 ";
                    searchCriteriaDisplay += "标签组匹配 (全部): \"" + csv_tags + "\"";
                    tagQueryProvided = true;
                }
            }
            else if (result.count("tag"))
            {
                std::string single_tag = result["tag"].as<std::string>();
                if (!single_tag.empty())
                {
                    tagsToFilterBy.push_back(single_tag);
                    matchAllTags = false; // Single tag implies OR logic if combined with other types of searches, but here it's the primary tag filter
                    if (nameQueryProvided)
                        searchCriteriaDisplay += " 并且 ";
                    searchCriteriaDisplay += "标签包含: \"" + single_tag + "\"";
                    tagQueryProvided = true;
                }
            }

            // Apply tag filtering if tags were specified
            if (tagQueryProvided && !tagsToFilterBy.empty())
            {
                std::vector<RecipeApp::Recipe> tagFilteredResults;
                for (const auto &recipe : recipesToDisplay) // Filter the (potentially name-filtered) list
                {
                    const auto &recipeTags = recipe.getTags();
                    bool currentRecipeMatch;
                    if (matchAllTags)
                    {
                        currentRecipeMatch = std::all_of(tagsToFilterBy.begin(), tagsToFilterBy.end(),
                                                         [&](const std::string &t)
                                                         {
                                                             return std::find(recipeTags.begin(), recipeTags.end(), t) != recipeTags.end();
                                                         });
                    }
                    else // Only for single --tag scenario
                    {
                        currentRecipeMatch = std::any_of(tagsToFilterBy.begin(), tagsToFilterBy.end(),
                                                         [&](const std::string &t)
                                                         {
                                                             return std::find(recipeTags.begin(), recipeTags.end(), t) != recipeTags.end();
                                                         });
                    }
                    if (currentRecipeMatch)
                    {
                        tagFilteredResults.push_back(recipe);
                    }
                }
                recipesToDisplay = tagFilteredResults; // Update the list to display
            }

            // Check if any search criteria was actually provided
            if (!nameQueryProvided && !tagQueryProvided)
            {
                // This means --recipe-search was called without a query, and no --tag or --tags.
                // Or just --recipe-search with an empty string.
                // The main.cpp option definition should ideally prevent --recipe-search without any value.
                // If it's allowed, it implies "list all", but that's `recipe-list`.
                // For a "search" command, some criteria should be present.
                if (result.count("recipe-search"))
                { // Check if the command itself was recipe-search
                    std::cerr << "错误：请为搜索提供查询词或标签。" << std::endl;
                    std::cerr << "用法: recipe-cli --recipe-search [查询词] [--tag <标签>] [--tags <标签1,标签2>]" << std::endl;
                    return RecipeApp::Cli::EX_USAGE;
                }
                // If not even --recipe-search command, this handler shouldn't be called.
                // But if it is, and no criteria, it's an issue.
                // For safety, if searchCriteriaDisplay is empty, list all.
                if (searchCriteriaDisplay.empty())
                    searchCriteriaDisplay = "所有菜谱 (无有效过滤器)";
            }

            std::cout << "--- 菜谱搜索结果 (" << searchCriteriaDisplay << ") ---" << std::endl;

            if (recipesToDisplay.empty())
            {
                std::cout << "未找到匹配的菜谱。" << std::endl;
            }
            else
            {
                for (const auto &recipe : recipesToDisplay)
                {
                    RecipeApp::CliUtils::displayRecipeDetailsBrief(recipe);
                }
                std::cout << "找到 " << recipesToDisplay.size() << " 个匹配的菜谱。" << std::endl;
            }
            return RecipeApp::Cli::EX_OK;
        }

        int RecipeCommandHandler::handleUpdateRecipe(const cxxopts::ParseResult &result)
        {
            if (!result.count("recipe-update"))
            {
                std::cerr << "错误：recipe-update 命令缺少参数 (RECIPE_ID)。" << std::endl;
                std::cerr << "用法: recipe-cli --recipe-update <菜谱ID>" << std::endl;
                return RecipeApp::Cli::EX_USAGE;
            }

            int recipeId = 0;
            try
            {
                recipeId = result["recipe-update"].as<int>();
            }
            catch (const cxxopts::exceptions::exception &e)
            {
                (void)e;
                std::cerr << "错误：无效的菜谱ID '" << result["recipe-update"].as<std::string>() << "'。请输入一个数字。" << std::endl;
                return RecipeApp::Cli::EX_DATAERR;
            }

            std::optional<RecipeApp::Recipe> recipeToUpdateOpt = recipeManager.findRecipeById(recipeId);
            if (!recipeToUpdateOpt)
            {
                std::cerr << "错误：未找到ID为 " << recipeId << " 的菜谱。" << std::endl;
                return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
            }
            RecipeApp::Recipe recipeToUpdate = recipeToUpdateOpt.value();

            std::cout << "--- 更新菜谱 (ID: " << recipeId << ") ---" << std::endl;
            std::cout << "当前菜谱信息：" << std::endl;
            RecipeApp::CliUtils::displayRecipeDetailsFull(recipeToUpdate);
            std::cout << "请输入新的菜谱信息 (留空则表示保留当前值)：" << std::endl;

            std::string newName = RecipeApp::CliUtils::getStringFromConsole("新名称 [" + recipeToUpdate.getName() + "]: ");
            if (!newName.empty())
            {
                if (newName != recipeToUpdate.getName())
                {
                    std::vector<RecipeApp::Recipe> existing = recipeManager.findRecipeByName(newName, false);
                    bool conflict = false;
                    for (const auto &r : existing)
                    {
                        if (r.getRecipeId() != recipeId)
                        {
                            conflict = true;
                            break;
                        }
                    }
                    if (conflict)
                    {
                        std::cerr << "错误：新菜谱名称 '" << newName << "' 已被其他菜谱使用。" << std::endl;
                        return RecipeApp::Cli::EX_APP_ALREADY_EXISTS;
                    }
                }
                recipeToUpdate.setName(newName);
            }

            std::cout << "修改配料? (y/n, 当前 " << recipeToUpdate.getIngredients().size() << " 项): ";
            std::string changeIngredients = RecipeApp::CliUtils::getStringFromConsole("");
            if (changeIngredients == "y" || changeIngredients == "Y")
            {
                recipeToUpdate.setIngredients(RecipeApp::CliUtils::getIngredientsFromConsole());
            }

            std::cout << "修改步骤? (y/n, 当前 " << recipeToUpdate.getSteps().size() << " 项): ";
            std::string changeSteps = RecipeApp::CliUtils::getStringFromConsole("");
            if (changeSteps == "y" || changeSteps == "Y")
            {
                recipeToUpdate.setSteps(RecipeApp::CliUtils::getStepsFromConsole());
            }

            // Update Tags
            std::string currentTagsDisplay;
            const auto &currentTags = recipeToUpdate.getTags();
            for (size_t i = 0; i < currentTags.size(); ++i)
            {
                currentTagsDisplay += currentTags[i] + (i == currentTags.size() - 1 ? "" : ",");
            }
            std::string new_tags_csv;
            if (result.count("tags"))
            { // Check if --tags option was provided via command line for update
                new_tags_csv = result["tags"].as<std::string>();
                // If --tags is provided, we use it directly, otherwise we'd prompt.
                // For non-interactive update via CLI flags, we assume the flag is the source of truth.
                recipeToUpdate.setTags(RecipeApp::CliUtils::parseCsvStringToVector(new_tags_csv));
            }
            else
            {
                // Interactive update for tags
                std::cout << "修改标签? (y/n, 当前: " << (currentTagsDisplay.empty() ? "无" : currentTagsDisplay) << "): ";
                std::string changeTagsStr = RecipeApp::CliUtils::getStringFromConsole("");
                if (changeTagsStr == "y" || changeTagsStr == "Y")
                {
                    new_tags_csv = RecipeApp::CliUtils::getStringFromConsole("新标签 (多个用逗号分隔, 输入空行以清除所有标签) [" + currentTagsDisplay + "]: ");
                    recipeToUpdate.setTags(RecipeApp::CliUtils::parseCsvStringToVector(new_tags_csv));
                }
            }

            std::string newTimeStr = RecipeApp::CliUtils::getStringFromConsole("新烹饪时长 (分钟) [" + std::to_string(recipeToUpdate.getCookingTime()) + "]: ");
            if (!newTimeStr.empty())
            {
                try
                {
                    int newTime = std::stoi(newTimeStr);
                    if (newTime > 0)
                        recipeToUpdate.setCookingTime(newTime);
                    else
                        std::cout << "输入的烹饪时长无效。值将保持不变。" << std::endl;
                }
                catch (const std::exception &e)
                {
                    (void)e;
                    std::cout << "输入的烹饪时长不是有效数字。值将保持不变。" << std::endl;
                }
            }

            std::cout << "修改难度? (y/n): ";
            std::string changeDifficulty = RecipeApp::CliUtils::getStringFromConsole("");
            if (changeDifficulty == "y" || changeDifficulty == "Y")
            {
                recipeToUpdate.setDifficulty(RecipeApp::CliUtils::getDifficultyFromConsole());
            }

            std::string newCuisine = RecipeApp::CliUtils::getStringFromConsole("新菜系 [" + recipeToUpdate.getCuisine() + "]: ");
            if (!newCuisine.empty())
                recipeToUpdate.setCuisine(newCuisine);

            std::string newNutritionalInfo = RecipeApp::CliUtils::getStringFromConsole("新营养信息 [" + recipeToUpdate.getNutritionalInfo().value_or("") + "]: ");
            if (!newNutritionalInfo.empty())
                recipeToUpdate.setNutritionalInfo(newNutritionalInfo);
            else
            {
                std::string clearInfo = RecipeApp::CliUtils::getStringFromConsole("清除营养信息? (y/n): ");
                if (clearInfo == "y" || clearInfo == "Y")
                    recipeToUpdate.setNutritionalInfo({});
            }

            std::string newImageUrl = RecipeApp::CliUtils::getStringFromConsole("新图片链接 [" + recipeToUpdate.getImageUrl().value_or("") + "]: ");
            if (!newImageUrl.empty())
                recipeToUpdate.setImageUrl(newImageUrl);
            else
            {
                std::string clearUrl = RecipeApp::CliUtils::getStringFromConsole("清除图片链接? (y/n): ");
                if (clearUrl == "y" || clearUrl == "Y")
                    recipeToUpdate.setImageUrl({});
            }

            if (recipeManager.updateRecipe(recipeToUpdate))
            {
                std::cout << "菜谱 ID " << recipeId << " 更新成功！" << std::endl;
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "更新菜谱失败。" << std::endl;
                return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
            }
        }

        int RecipeCommandHandler::handleDeleteRecipe(const cxxopts::ParseResult &result)
        {
            // const RecipeApp::User *currentUser = userManager.getCurrentUser(); // No longer needed for auth check
            // if (!currentUser) // This check is no longer needed
            // {
            //     std::cerr << "Error: Please login first to delete a recipe." << std::endl;
            //     return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            // }

            // if (currentUser->getRole() != RecipeApp::UserRole::Admin) // This check is no longer needed
            // {
            //     std::cerr << "Error: Permission denied. Only administrators can delete recipes." << std::endl;
            //     return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            // }

            if (!result.count("recipe-delete"))
            {
                std::cerr << "错误：recipe-delete 命令缺少参数 (RECIPE_ID)。" << std::endl;
                std::cerr << "用法: recipe-cli --recipe-delete <菜谱ID>" << std::endl;
                return RecipeApp::Cli::EX_USAGE;
            }

            int recipeId = 0;
            try
            {
                recipeId = result["recipe-delete"].as<int>();
            }
            catch (const cxxopts::exceptions::exception &e)
            {
                (void)e; // Mark as intentionally unused
                std::cerr << "错误：无效的菜谱ID '" << result["recipe-delete"].as<std::string>() << "'。请输入一个数字。" << std::endl;
                return RecipeApp::Cli::EX_DATAERR;
            }

            std::optional<RecipeApp::Recipe> recipeToDeleteOpt = recipeManager.findRecipeById(recipeId);
            if (!recipeToDeleteOpt)
            {
                std::cerr << "错误：未找到ID为 " << recipeId << " 的菜谱。" << std::endl;
                return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
            }

            std::cout << "找到菜谱: " << recipeToDeleteOpt.value().getName() << " (ID: " << recipeId << ")" << std::endl;
            std::string confirm = RecipeApp::CliUtils::getStringFromConsole("您确定要删除这个菜谱吗？ (y/n): ");

            if (confirm == "y" || confirm == "Y")
            {
                if (recipeManager.deleteRecipe(recipeId))
                {
                    std::cout << "菜谱 ID " << recipeId << " 删除成功！" << std::endl;
                    return RecipeApp::Cli::EX_OK;
                }
                else
                {
                    std::cerr << "删除菜谱失败。" << std::endl;
                    return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
                }
            }
            else
            {
                std::cout << "删除操作已取消。" << std::endl;
                return RecipeApp::Cli::EX_OK; // User cancelled, not an error
            }
        }

    } // namespace CliHandlers
} // namespace RecipeApp