#include "RecipeEncyclopediaCommandHandler.h"
#include "cli/CliUtils.h"
#include "cli/ExitCodes.h"
#include "domain/recipe/Recipe.h"
#include "../../common/exceptions/ValidationException.h" // For ValidationException
#include "spdlog/spdlog.h" // For logging
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

namespace RecipeApp
{
    namespace CliHandlers
    {
        RecipeEncyclopediaCommandHandler::RecipeEncyclopediaCommandHandler(RecipeApp::Logic::Encyclopedia::RecipeEncyclopediaManager &manager)
            : encyclopediaManager(manager) {}

        int RecipeEncyclopediaCommandHandler::handleSearchEncyclopediaRecipes(const cxxopts::ParseResult &result)
        {
            if (!result.count("enc-search")) {
                spdlog::error("处理食谱大全搜索错误：缺少 --enc-search 选项。");
                throw Common::Exceptions::ValidationException("搜索食谱大全需要 --enc-search 选项和关键词。");
            }
            
            std::string keywords;
            try {
                keywords = result["enc-search"].as<std::string>();
            } catch (const cxxopts::exceptions::exception& e) {
                // This might happen if --enc-search is present but somehow cxxopts fails to cast (e.g. if it was defined differently)
                // However, main.cpp defines it as string, so this is less likely for simple presence.
                // More likely if the option was allowed to be valueless and then as<string>() is called.
                spdlog::error("处理食谱大全搜索错误：无法将 --enc-search 的值解析为字符串。原始错误: {}", e.what());
                throw Common::Exceptions::ValidationException("无法解析 --enc-search 选项的值。");
            }

            if (keywords.empty()) {
                spdlog::error("处理食谱大全搜索错误：--enc-search 选项的关键词不能为空。");
                throw Common::Exceptions::ValidationException("搜索食谱大全的关键词不能为空。");
            }
            
            spdlog::debug("在食谱大全中搜索关键词: '{}'", keywords);
            std::vector<RecipeApp::Recipe> recipes = encyclopediaManager.searchRecipes(keywords);

            if (recipes.empty())
            {
                std::cout << "未找到与关键词匹配的食谱: '" << keywords << "'." << std::endl;
            }
            else
            {
                std::cout << "找到 " << recipes.size() << " 个与关键词匹配的食谱 '" << keywords << "':" << std::endl;
                for (const auto &recipe : recipes)
                {
                    std::cout << "  ID: " << recipe.getRecipeId() << ", 名称: " << recipe.getName() << std::endl;
                    // TODO: Implement a more detailed summary if needed, e.g., using a CliUtils function
                    // RecipeApp::CliUtils::displayRecipeDetailsBrief(recipe);
                }
            }
            return RecipeApp::Cli::EX_OK;
        }

        int RecipeEncyclopediaCommandHandler::handleViewEncyclopediaRecipe(const cxxopts::ParseResult &result)
        {
            if (!result.count("enc-view")) {
                spdlog::error("处理查看百科食谱错误：缺少 --enc-view 选项。");
                throw Common::Exceptions::ValidationException("查看百科食谱需要 --enc-view 选项和ID。");
            }

            int recipeId = 0;
            try {
                recipeId = result["enc-view"].as<int>();
                if (recipeId <= 0) {
                    spdlog::error("处理查看百科食谱错误：提供的ID '{}' 无效，ID必须为正整数。", recipeId);
                    throw Common::Exceptions::ValidationException("百科食谱ID必须为正整数。");
                }
            } catch (const cxxopts::exceptions::exception& e) {
                spdlog::error("处理查看百科食谱错误：无法将 --enc-view 的值解析为整数。原始错误: {}", e.what());
                throw Common::Exceptions::ValidationException(std::string("无效的百科食谱ID格式: ") + e.what());
            }
            
            spdlog::debug("查看百科食谱，ID: {}", recipeId);
            std::optional<RecipeApp::Recipe> recipeOpt = encyclopediaManager.getRecipeById(recipeId);

            if (recipeOpt)
            {
                // TODO: Consider using a CliUtils function for consistent display, e.g., RecipeApp::CliUtils::displayRecipeDetailsFull(*recipeOpt);
                std::cout << "--- 食谱详情 (来自百科) ---" << std::endl;
                std::cout << "ID: " << recipeOpt->getRecipeId() << std::endl;
                std::cout << "名称: " << recipeOpt->getName() << std::endl;
                std::cout << "烹饪时长: " << recipeOpt->getCookingTime() << " 分钟" << std::endl;
                std::cout << "难度: " << RecipeApp::CliUtils::difficultyToString(recipeOpt->getDifficulty()) << std::endl;
                
                std::cout << "配料:" << std::endl;
                if (recipeOpt->getIngredients().empty()) {
                    std::cout << "  (无配料信息)" << std::endl;
                } else {
                    for (const auto &ing : recipeOpt->getIngredients()) {
                        std::cout << "  - " << ing.name << " (" << ing.quantity << ")" << std::endl;
                    }
                }

                std::cout << "步骤:" << std::endl;
                if (recipeOpt->getSteps().empty()) {
                    std::cout << "  (无步骤信息)" << std::endl;
                } else {
                    int stepNum = 1;
                    for (const auto &step : recipeOpt->getSteps()) {
                        std::cout << "  " << stepNum++ << ". " << step << std::endl;
                    }
                }
                // Display Tags
                std::cout << "标签:" << std::endl;
                if (recipeOpt->getTags().empty()) {
                    std::cout << "  (无标签信息)" << std::endl;
                } else {
                    std::string tags_str;
                    const auto &tags = recipeOpt->getTags();
                    for (size_t i = 0; i < tags.size(); ++i) {
                        tags_str += tags[i];
                        if (i < tags.size() - 1) {
                            tags_str += ", ";
                        }
                    }
                    std::cout << "  " << tags_str << std::endl;
                }
                if(recipeOpt->getNutritionalInfo().has_value() && !recipeOpt->getNutritionalInfo().value().empty()){
                    std::cout << "营养信息: " << recipeOpt->getNutritionalInfo().value() << std::endl;
                }
                 if(recipeOpt->getImageUrl().has_value() && !recipeOpt->getImageUrl().value().empty()){
                    std::cout << "图片链接: " << recipeOpt->getImageUrl().value() << std::endl;
                }
                std::cout << "--------------------------" << std::endl;

            }
            else
            {
                // This is not an error from a validation perspective, but an application "not found" state.
                // The command itself was valid.
                spdlog::info("未在百科中找到ID为 {} 的食谱。", recipeId);
                std::cout << "未在百科中找到ID为 " << recipeId << " 的食谱。" << std::endl;
            }
            return RecipeApp::Cli::EX_OK;
        }
    } // namespace CliHandlers
} // namespace RecipeApp