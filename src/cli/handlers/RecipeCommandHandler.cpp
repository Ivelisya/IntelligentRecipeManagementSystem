#include "cli/handlers/RecipeCommandHandler.h"
#include "cli/CliUtils.h"
#include "cli/ExitCodes.h"         // For standardized exit codes
#include "domain/recipe/Recipe.h"  // For RecipeApp::Difficulty
#include "domain/user/User.h"      // For RecipeApp::UserRole
#include "core/CustomLinkedList.h" // For CustomLinkedList type
#include <iostream>
#include <string>
#include <vector>
#include <limits>    // Required for std::numeric_limits
#include <stdexcept> // Required for std::exception

namespace RecipeApp
{
    namespace CliHandlers
    {

        RecipeCommandHandler::RecipeCommandHandler(RecipeApp::RecipeManager &rm, RecipeApp::UserManager &um)
            : recipeManager(rm), userManager(um) {}

        int RecipeCommandHandler::handleAddRecipe(const cxxopts::ParseResult &result)
        {
            // const RecipeApp::User *currentUser = userManager.getCurrentUser(); // No longer needed for auth check
            // if (!currentUser) // This check is no longer needed as user is always admin
            // {
            //     std::cerr << "Error: Please login first to add a recipe." << std::endl;
            //     return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            // }

            std::cout << "--- 添加新菜谱 ---" << std::endl;
            std::string name = RecipeApp::CliUtils::getStringFromConsole("请输入菜谱名称: ");
            if (name.empty())
            {
                std::cerr << "错误：菜谱名称不能为空。" << std::endl;
                return RecipeApp::Cli::EX_APP_INVALID_INPUT;
            }

            CustomDataStructures::CustomLinkedList<RecipeApp::Recipe> existingRecipes = recipeManager.findRecipeByName(name, false);
            if (!existingRecipes.isEmpty())
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
            std::string nutritionalInfo = RecipeApp::CliUtils::getStringFromConsole("请输入营养信息 (可选, 可为空): ");
            std::string imageUrl = RecipeApp::CliUtils::getStringFromConsole("请输入图片链接 (可选, 可为空): ");

            RecipeApp::Recipe newRecipe(0, name, ingredients, steps, cookingTime, difficulty, cuisine);
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
            if (!result.count("recipe-search"))
            {
                std::cerr << "错误：recipe-search 命令缺少参数 (QUERY)。" << std::endl;
                std::cerr << "用法: recipe-cli --recipe-search <查询词>" << std::endl;
                return RecipeApp::Cli::EX_USAGE;
            }
            std::string query = result["recipe-search"].as<std::string>();
            if (query.empty())
            {
                std::cerr << "错误：搜索查询词不能为空。" << std::endl;
                return RecipeApp::Cli::EX_APP_INVALID_INPUT;
            }
            std::cout << "--- 菜谱搜索结果 (名称包含: \"" << query << "\") ---" << std::endl;

            CustomDataStructures::CustomLinkedList<RecipeApp::Recipe> searchResults = recipeManager.findRecipeByName(query, true);

            if (searchResults.isEmpty())
            {
                std::cout << "未找到名称包含 \"" << query << "\" 的菜谱。" << std::endl;
            }
            else
            {
                for (const auto &recipe : searchResults)
                {
                    RecipeApp::CliUtils::displayRecipeDetailsBrief(recipe);
                }
                std::cout << "找到 " << searchResults.getSize() << " 个匹配的菜谱。" << std::endl;
            }
            return RecipeApp::Cli::EX_OK; // Search itself is successful even if no results
        }

        int RecipeCommandHandler::handleUpdateRecipe(const cxxopts::ParseResult &result)
        {
            // const RecipeApp::User *currentUser = userManager.getCurrentUser(); // No longer needed for auth check
            // if (!currentUser) // This check is no longer needed
            // {
            //     std::cerr << "Error: Please login first to update a recipe." << std::endl;
            //     return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            // }

            // if (currentUser->getRole() != RecipeApp::UserRole::Admin) // This check is no longer needed
            // {
            //     std::cerr << "Error: Permission denied. Only administrators can update recipes." << std::endl;
            //     return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            // }

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
                std::cerr << "错误：无效的菜谱ID '" << result["recipe-update"].as<std::string>() << "'。请输入一个数字。" << std::endl;
                return RecipeApp::Cli::EX_DATAERR;
            }

            std::optional<RecipeApp::Recipe> recipeToUpdateOpt = recipeManager.findRecipeById(recipeId);
            if (!recipeToUpdateOpt)
            {
                std::cerr << "错误：未找到ID为 " << recipeId << " 的菜谱。" << std::endl;
                return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
            }
            RecipeApp::Recipe recipeToUpdate = recipeToUpdateOpt.value(); // Work with a mutable copy

            std::cout << "--- 更新菜谱 (ID: " << recipeId << ") ---" << std::endl;
            std::cout << "当前菜谱信息：" << std::endl;
            RecipeApp::CliUtils::displayRecipeDetailsFull(recipeToUpdate);
            std::cout << "请输入新的菜谱信息 (留空则表示保留当前值)：" << std::endl;

            std::string newName = RecipeApp::CliUtils::getStringFromConsole("新名称 [" + recipeToUpdate.getName() + "]: ");
            if (!newName.empty())
            {
                if (newName != recipeToUpdate.getName())
                {
                    CustomDataStructures::CustomLinkedList<RecipeApp::Recipe> existing = recipeManager.findRecipeByName(newName, false);
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