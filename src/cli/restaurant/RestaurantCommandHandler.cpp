#include "RestaurantCommandHandler.h"
#include "spdlog/spdlog.h"
#include "cli/ExitCodes.h"
#include "cli/CliUtils.h"
#include "domain/restaurant/Restaurant.h"
#include "../../common/exceptions/ValidationException.h"
#include "../../common/exceptions/PersistenceException.h"
#include <iostream>
#include <string>
#include <vector>
#include <optional>

namespace RecipeApp
{
    namespace CliHandlers
    {
        RestaurantCommandHandler::RestaurantCommandHandler(RecipeApp::RestaurantManager &rm, RecipeApp::RecipeManager &recipeMgr)
            : restaurantManager_(rm), recipeManager_(recipeMgr) {}

        int RestaurantCommandHandler::handleAddRestaurant(const cxxopts::ParseResult& result) {
            spdlog::debug("处理添加餐馆命令。");
            std::cout << "--- 添加新餐馆 ---" << std::endl;

            try {
                std::string name = RecipeApp::CliUtils::getStringFromConsole("请输入餐馆名称: ");
                if (name.empty()) {
                    spdlog::error("餐馆名称不能为空。");
                    throw RecipeApp::Common::Exceptions::ValidationException("餐馆名称不能为空。");
                }

                std::string address = RecipeApp::CliUtils::getStringFromConsole("请输入餐馆地址: ");
                if (address.empty()) {
                    spdlog::error("餐馆地址不能为空。");
                    throw RecipeApp::Common::Exceptions::ValidationException("餐馆地址不能为空。");
                }

                std::string contact = RecipeApp::CliUtils::getStringFromConsole("请输入联系方式: ");
                if (contact.empty()) {
                    spdlog::error("餐馆联系方式不能为空。");
                    throw RecipeApp::Common::Exceptions::ValidationException("餐馆联系方式不能为空。");
                }

                std::string openingHours = RecipeApp::CliUtils::getStringFromConsole("请输入营业时间 (可选): ");

                std::string featuredRecipeIdsStr = RecipeApp::CliUtils::getStringFromConsole(
                    "请输入特色菜谱ID列表 (可选, 逗号分隔): ");
                std::vector<int> featuredRecipeIds;
                if (!featuredRecipeIdsStr.empty()) {
                    try {
                        featuredRecipeIds = RecipeApp::CliUtils::parseCsvStringToIntVector(featuredRecipeIdsStr);
                    } catch (const std::invalid_argument& ia) {
                        spdlog::error("解析特色菜谱ID列表失败: {}", ia.what());
                        throw RecipeApp::Common::Exceptions::ValidationException("特色菜谱ID列表格式无效。");
                    } catch (const std::out_of_range& oor) {
                        spdlog::error("解析特色菜谱ID列表时发生溢出: {}", oor.what());
                        throw RecipeApp::Common::Exceptions::ValidationException("特色菜谱ID数值过大。");
                    }
                }

                RecipeApp::RestaurantBuilder builder = RecipeApp::Restaurant::builder(0, name);
                builder.withAddress(address)
                       .withContact(contact);

                if (!openingHours.empty()) {
                    builder.withOpeningHours(openingHours);
                }
                if (!featuredRecipeIds.empty()) {
                    builder.withFeaturedRecipeIds(featuredRecipeIds);
                }

                RecipeApp::Restaurant newRestaurant = builder.build(); 

                int newId = restaurantManager_.addRestaurant(newRestaurant);
                
                if (newId != -1) {
                    spdlog::info("餐馆 '{}' 添加成功，ID: {}.", name, newId);
                    std::cout << "餐馆 '" << name << "' 添加成功！ (新 ID: " << newId << ")" << std::endl;
                    return RecipeApp::Cli::EX_OK;
                } else {
                    spdlog::error("添加餐馆 '{}' 失败 (管理器返回错误代码)。", name);
                    std::cout << "添加餐馆失败。请检查日志。" << std::endl;
                    return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
                }

            } catch (const RecipeApp::Common::Exceptions::ValidationException& ve) {
                std::cout << "错误: " << ve.what() << std::endl; 
                return RecipeApp::Cli::EX_DATAERR; 
            } catch (const RecipeApp::Common::Exceptions::PersistenceException& pe) {
                std::cout << "错误: " << pe.what() << std::endl;
                return RecipeApp::Cli::EX_IOERR;
            } catch (const std::exception& e) { 
                spdlog::error("添加餐馆时发生未知错误: {}", e.what());
                std::cout << "发生意外错误: " << e.what() << std::endl;
                return RecipeApp::Cli::EX_SOFTWARE;
            }
        }

        int RestaurantCommandHandler::handleListRestaurants(const cxxopts::ParseResult& result) {
            spdlog::debug("处理列出餐馆命令。");
            std::cout << "--- 餐馆列表 ---" << std::endl;
            try {
                std::vector<RecipeApp::Restaurant> restaurants = restaurantManager_.getAllRestaurants();
                if (restaurants.empty()) {
                    std::cout << "当前没有已保存的餐馆。" << std::endl;
                } else {
                    for (const auto& r : restaurants) {
                        std::cout << "ID: " << r.getId() 
                                  << ", 名称: " << r.getName()
                                  << ", 地址: " << r.getAddress()
                                  << ", 联系方式: " << r.getContact()
                                  << std::endl;
                    }
                    std::cout << "共 " << restaurants.size() << " 个餐馆。" << std::endl;
                }
                return RecipeApp::Cli::EX_OK;
            } catch (const RecipeApp::Common::Exceptions::PersistenceException& pe) {
                spdlog::error("获取餐馆列表时发生持久化错误: {}", pe.what());
                std::cout << "错误: 获取餐馆列表失败 - " << pe.what() << std::endl;
                return RecipeApp::Cli::EX_IOERR;
            } catch (const std::exception& e) {
                spdlog::error("获取餐馆列表时发生未知错误: {}", e.what());
                std::cout << "发生意外错误: " << e.what() << std::endl;
                return RecipeApp::Cli::EX_SOFTWARE;
            }
        }

        int RestaurantCommandHandler::handleViewRestaurant(const cxxopts::ParseResult& result) {
            spdlog::debug("处理查看餐馆命令。");
            if (!result.count("restaurant-view")) {
                 spdlog::error("查看餐馆命令 (--restaurant-view) 需要一个餐馆ID参数。");
                 throw RecipeApp::Common::Exceptions::ValidationException("查看餐馆需要提供餐馆ID。");
            }

            int restaurantId = 0;
            try {
                restaurantId = result["restaurant-view"].as<int>();
                if (restaurantId <= 0) {
                    spdlog::error("提供的餐馆ID '{}' 无效，ID必须为正整数。", restaurantId);
                    throw RecipeApp::Common::Exceptions::ValidationException("无效的餐馆ID: ID必须为正整数。");
                }
            } catch (const cxxopts::exceptions::exception& e) { 
                spdlog::error("解析餐馆ID时出错: {}. 提供的参数: '{}'.", e.what(), result["restaurant-view"].as<std::string>());
                throw RecipeApp::Common::Exceptions::ValidationException("无效的餐馆ID格式，请输入一个数字。");
            }

            try {
                std::optional<RecipeApp::Restaurant> restaurantOpt = restaurantManager_.findRestaurantById(restaurantId);
                if (restaurantOpt) {
                    const auto& r = restaurantOpt.value();
                    std::cout << "--- 餐馆详情 (ID: " << r.getId() << ") ---" << std::endl;
                    std::cout << "名称: " << r.getName() << std::endl;
                    std::cout << "地址: " << r.getAddress() << std::endl;
                    std::cout << "联系方式: " << r.getContact() << std::endl;
                    std::cout << "营业时间: " << (r.getOpeningHours().empty() ? "(未提供)" : r.getOpeningHours()) << std::endl;
                    
                    std::cout << "特色菜谱:" << std::endl;
                    // Use getFeaturedRecipes to fetch actual Recipe objects
                    std::vector<RecipeApp::Recipe> featuredRecipes = restaurantManager_.getFeaturedRecipes(r.getId(), recipeManager_);
                    if (!featuredRecipes.empty()) {
                        for (const auto& recipe : featuredRecipes) {
                            std::cout << "  - ID: " << recipe.getRecipeId() << ", 名称: " << recipe.getName() << std::endl;
                        }
                    } else {
                        std::cout << "  (无特色菜谱)" << std::endl;
                    }
                    std::cout << "-------------------------" << std::endl;
                    return RecipeApp::Cli::EX_OK;
                } else {
                    spdlog::warn("未找到ID为 {} 的餐馆。", restaurantId);
                    std::cout << "未找到ID为 " << restaurantId << " 的餐馆。" << std::endl;
                    return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
                }
            } catch (const RecipeApp::Common::Exceptions::PersistenceException& pe) {
                spdlog::error("查看餐馆 ID {} 时发生持久化错误: {}", restaurantId, pe.what());
                std::cout << "错误: 查看餐馆失败 - " << pe.what() << std::endl;
                return RecipeApp::Cli::EX_IOERR;
            } catch (const std::exception& e) {
                spdlog::error("查看餐馆 ID {} 时发生未知错误: {}", restaurantId, e.what());
                std::cout << "发生意外错误: " << e.what() << std::endl;
                return RecipeApp::Cli::EX_SOFTWARE;
            }
        }

        int RestaurantCommandHandler::handleUpdateRestaurant(const cxxopts::ParseResult& result) {
            spdlog::debug("处理更新餐馆命令。");
            if (!result.count("restaurant-update")) {
                 spdlog::error("更新餐馆命令 (--restaurant-update) 需要一个餐馆ID参数。");
                 throw RecipeApp::Common::Exceptions::ValidationException("更新餐馆需要提供餐馆ID。");
            }

            int restaurantId = 0;
            try {
                restaurantId = result["restaurant-update"].as<int>();
                if (restaurantId <= 0) {
                    spdlog::error("提供的餐馆ID '{}' 无效，ID必须为正整数。", restaurantId);
                    throw RecipeApp::Common::Exceptions::ValidationException("无效的餐馆ID: ID必须为正整数。");
                }
            } catch (const cxxopts::exceptions::exception& e) {
                spdlog::error("解析餐馆ID时出错: {}. 提供的参数: '{}'.", e.what(), result["restaurant-update"].as<std::string>());
                throw RecipeApp::Common::Exceptions::ValidationException("无效的餐馆ID格式，请输入一个数字。");
            }

            try {
                std::optional<RecipeApp::Restaurant> restaurantOpt = restaurantManager_.findRestaurantById(restaurantId);
                if (!restaurantOpt) {
                    spdlog::warn("尝试更新但未找到ID为 {} 的餐馆。", restaurantId);
                    std::cout << "错误：未找到ID为 " << restaurantId << " 的餐馆。" << std::endl;
                    return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
                }

                RecipeApp::Restaurant restaurantToUpdate = restaurantOpt.value();
                std::cout << "--- 更新餐馆 (ID: " << restaurantId << ") ---" << std::endl;
                std::cout << "当前信息:" << std::endl;
                std::cout << "  名称: " << restaurantToUpdate.getName() << std::endl;
                std::cout << "  地址: " << restaurantToUpdate.getAddress() << std::endl;
                std::cout << "  联系方式: " << restaurantToUpdate.getContact() << std::endl;
                std::cout << "  营业时间: " << restaurantToUpdate.getOpeningHours() << std::endl;
                std::cout << "请输入新信息 (留空则保留当前值):" << std::endl;

                std::string newName = RecipeApp::CliUtils::getStringFromConsole("新名称 [" + restaurantToUpdate.getName() + "]: ");
                if (!newName.empty()) {
                    restaurantToUpdate.setName(newName);
                }

                std::string newAddress = RecipeApp::CliUtils::getStringFromConsole("新地址 [" + restaurantToUpdate.getAddress() + "]: ");
                if (!newAddress.empty()) {
                    restaurantToUpdate.setAddress(newAddress);
                }

                std::string newContact = RecipeApp::CliUtils::getStringFromConsole("新联系方式 [" + restaurantToUpdate.getContact() + "]: ");
                if (!newContact.empty()) {
                    restaurantToUpdate.setContact(newContact);
                }
                
                std::string newOpeningHours_input = RecipeApp::CliUtils::getStringFromConsole("新营业时间 [" + restaurantToUpdate.getOpeningHours() + "]: ");
                if (!newOpeningHours_input.empty()) {
                    restaurantToUpdate.setOpeningHours(newOpeningHours_input);
                } else {
                    std::string currentOpeningHoursDisplay = restaurantToUpdate.getOpeningHours().empty() ? "空" : "'" + restaurantToUpdate.getOpeningHours() + "'";
                    std::string clearOpeningHours = RecipeApp::CliUtils::getStringFromConsole("要清除营业时间吗 (当前: " + currentOpeningHoursDisplay + ")? (y/n): ");
                    if (clearOpeningHours == "y" || clearOpeningHours == "Y") {
                        restaurantToUpdate.setOpeningHours("");
                    }
                }

                std::string currentFeaturedIdsStr;
                const auto& currentIds = restaurantToUpdate.getFeaturedRecipeIds();
                for(size_t i=0; i < currentIds.size(); ++i){
                    currentFeaturedIdsStr += std::to_string(currentIds[i]);
                    if(i < currentIds.size() -1) currentFeaturedIdsStr += ",";
                }
                std::string newFeaturedRecipeIdsStr = RecipeApp::CliUtils::getStringFromConsole(
                    "新特色菜谱ID列表 (逗号分隔) [" + (currentFeaturedIdsStr.empty() ? "无" : currentFeaturedIdsStr) + "]: ");
                
                if (!newFeaturedRecipeIdsStr.empty()) {
                    try {
                        restaurantToUpdate.setFeaturedRecipeIds(RecipeApp::CliUtils::parseCsvStringToIntVector(newFeaturedRecipeIdsStr));
                    } catch (const std::invalid_argument& ia) {
                        spdlog::error("解析新特色菜谱ID列表失败: {}", ia.what());
                        throw RecipeApp::Common::Exceptions::ValidationException("新特色菜谱ID列表格式无效。");
                    } catch (const std::out_of_range& oor) {
                        spdlog::error("解析新特色菜谱ID列表时发生溢出: {}", oor.what());
                        throw RecipeApp::Common::Exceptions::ValidationException("新特色菜谱ID数值过大。");
                    }
                } else {
                     std::string clearFeaturedIds = RecipeApp::CliUtils::getStringFromConsole("要清除所有特色菜谱ID吗 (当前: " + (currentFeaturedIdsStr.empty() ? "无" : currentFeaturedIdsStr) + ")? (y/n): ");
                     if (clearFeaturedIds == "y" || clearFeaturedIds == "Y") {
                        restaurantToUpdate.setFeaturedRecipeIds({});
                    }
                }

                // Try to update the restaurant object. Setters might throw std::invalid_argument.
                try {
                    // The following setters might throw std::invalid_argument if new value is invalid (e.g. empty for required field)
                    // This is based on Restaurant domain object's internal validation.
                    if (!newName.empty()) {
                        restaurantToUpdate.setName(newName);
                    }
                    if (!newAddress.empty()) {
                        restaurantToUpdate.setAddress(newAddress);
                    }
                    if (!newContact.empty()) {
                        restaurantToUpdate.setContact(newContact);
                    }
                    // Opening hours and featured IDs are already set or cleared based on user input.
                    // Their setters in Restaurant class might also have validation.
                } catch (const std::invalid_argument& ia) {
                    spdlog::error("更新餐馆属性时发生校验错误: {}", ia.what());
                    throw RecipeApp::Common::Exceptions::ValidationException(std::string("更新餐馆属性失败: ") + ia.what());
                }


                if (restaurantManager_.updateRestaurant(restaurantToUpdate)) {
                    spdlog::info("餐馆 ID {} 更新成功。", restaurantId);
                    std::cout << "餐馆 ID " << restaurantId << " 更新成功！" << std::endl;
                    return RecipeApp::Cli::EX_OK;
                } else {
                    // This case implies a business logic failure within the manager (e.g., name conflict)
                    // that wasn't thrown as a specific exception, or a persistence failure not caught as PersistenceException.
                    // Ideally, RestaurantManager::updateRestaurant should throw specific exceptions.
                    spdlog::error("更新餐馆 ID {} 失败 (管理器返回false)。可能原因：名称冲突或持久化层未抛出特定异常。", restaurantId);
                    std::cout << "更新餐馆失败。可能存在名称冲突或内部错误。请检查日志。" << std::endl;
                    return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
                }

            } catch (const RecipeApp::Common::Exceptions::ValidationException& ve) {
                std::cout << "错误 (校验): " << ve.what() << std::endl; // User-facing
                // spdlog::error already called by the thrower or the catch block above for std::invalid_argument
                return RecipeApp::Cli::EX_DATAERR;
            } catch (const RecipeApp::Common::Exceptions::PersistenceException& pe) {
                std::cout << "错误 (持久化): " << pe.what() << std::endl; // User-facing
                // spdlog::error should be called by RestaurantManager or Repository
                return RecipeApp::Cli::EX_IOERR;
            } catch (const std::exception& e) { // Catch-all for other unexpected issues from this level
                spdlog::error("更新餐馆 ID {} 时发生未知标准库错误: {}", restaurantId, e.what());
                std::cout << "发生意外错误: " << e.what() << std::endl;
                return RecipeApp::Cli::EX_SOFTWARE;
            }
        }

        int RestaurantCommandHandler::handleDeleteRestaurant(const cxxopts::ParseResult& result) {
            spdlog::debug("处理删除餐馆命令。");
            if (!result.count("restaurant-delete")) {
                 spdlog::error("删除餐馆命令 (--restaurant-delete) 需要一个餐馆ID参数。");
                 throw RecipeApp::Common::Exceptions::ValidationException("删除餐馆需要提供餐馆ID。");
            }
            
            int restaurantId = 0;
            try {
                restaurantId = result["restaurant-delete"].as<int>();
                if (restaurantId <= 0) {
                    spdlog::error("提供的餐馆ID '{}' 无效，ID必须为正整数。", restaurantId);
                    throw RecipeApp::Common::Exceptions::ValidationException("无效的餐馆ID: ID必须为正整数。");
                }
            } catch (const cxxopts::exceptions::exception& e) { 
                spdlog::error("解析餐馆ID时出错: {}. 提供的参数: '{}'.", e.what(), result["restaurant-delete"].as<std::string>());
                throw RecipeApp::Common::Exceptions::ValidationException("无效的餐馆ID格式，请输入一个数字。");
            }

            try {
                std::optional<RecipeApp::Restaurant> restaurantToDeleteOpt = restaurantManager_.findRestaurantById(restaurantId);
                if (!restaurantToDeleteOpt) {
                     spdlog::warn("尝试删除但未找到ID为 {} 的餐馆。", restaurantId);
                     std::cout << "错误：未找到ID为 " << restaurantId << " 的餐馆。" << std::endl;
                     return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
                }
                
                std::cout << "找到餐馆: " << restaurantToDeleteOpt.value().getName() 
                          << " (ID: " << restaurantId << ")" << std::endl;
                std::string confirm = RecipeApp::CliUtils::getStringFromConsole("您确定要删除这个餐馆吗？ (y/n): ");

                if (confirm == "y" || confirm == "Y") {
                    if (restaurantManager_.deleteRestaurant(restaurantId)) {
                        spdlog::info("ID为 {} 的餐馆已成功删除。", restaurantId);
                        std::cout << "餐馆 ID " << restaurantId << " 删除成功！" << std::endl;
                        return RecipeApp::Cli::EX_OK;
                    } else {
                        spdlog::error("删除ID为 {} 的餐馆失败 (管理器返回false)。", restaurantId);
                        std::cout << "删除餐馆失败。请检查日志。" << std::endl;
                        return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
                    }
                } else {
                    spdlog::info("删除操作已取消。");
                    std::cout << "删除操作已取消。" << std::endl;
                    return RecipeApp::Cli::EX_OK;
                }
            } catch (const RecipeApp::Common::Exceptions::PersistenceException& pe) {
                spdlog::error("删除餐馆 ID {} 时发生持久化错误: {}", restaurantId, pe.what());
                std::cout << "错误: 删除餐馆失败 - " << pe.what() << std::endl;
                return RecipeApp::Cli::EX_IOERR;
            } catch (const std::exception& e) {
                spdlog::error("删除餐馆 ID {} 时发生未知错误: {}", restaurantId, e.what());
                std::cout << "发生意外错误: " << e.what() << std::endl;
                return RecipeApp::Cli::EX_SOFTWARE;
            }
        }

        int RestaurantCommandHandler::handleManageRestaurantMenu(const cxxopts::ParseResult& result) {
            spdlog::debug("处理管理餐馆菜单命令。");
            if (!result.count("restaurant-manage-menu")) {
                 spdlog::error("管理餐馆菜单命令 (--restaurant-manage-menu) 需要一个餐馆ID参数。");
                 throw RecipeApp::Common::Exceptions::ValidationException("管理餐馆菜单需要提供餐馆ID。");
            }

            int restaurantId = 0;
            try {
                restaurantId = result["restaurant-manage-menu"].as<int>();
                if (restaurantId <= 0) {
                    spdlog::error("提供的餐馆ID '{}' 无效，ID必须为正整数。", restaurantId);
                    throw RecipeApp::Common::Exceptions::ValidationException("无效的餐馆ID: ID必须为正整数。");
                }
            } catch (const cxxopts::exceptions::exception& e) {
                spdlog::error("解析餐馆ID时出错: {}. 提供的参数: '{}'.", e.what(), result["restaurant-manage-menu"].as<std::string>());
                throw RecipeApp::Common::Exceptions::ValidationException("无效的餐馆ID格式，请输入一个数字。");
            }

            try {
                std::optional<RecipeApp::Restaurant> restaurantOpt = restaurantManager_.findRestaurantById(restaurantId);
                if (!restaurantOpt) {
                    spdlog::warn("尝试管理菜单但未找到ID为 {} 的餐馆。", restaurantId);
                    std::cout << "错误：未找到ID为 " << restaurantId << " 的餐馆。" << std::endl;
                    return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
                }

                RecipeApp::Restaurant restaurantToManage = restaurantOpt.value();
                std::cout << "--- 管理餐馆 '" << restaurantToManage.getName() << "' (ID: " << restaurantId << ") 的菜单 ---" << std::endl;
                
                std::cout << "当前特色菜谱:" << std::endl;
                std::vector<RecipeApp::Recipe> currentFeaturedRecipes = restaurantManager_.getFeaturedRecipes(restaurantId, recipeManager_);
                if (!currentFeaturedRecipes.empty()) {
                    for (const auto& recipe : currentFeaturedRecipes) {
                        std::cout << "  - ID: " << recipe.getRecipeId() << ", 名称: " << recipe.getName() << std::endl;
                    }
                } else {
                    std::cout << "  (当前菜单为空)" << std::endl;
                }
                std::cout << "-------------------------" << std::endl;

                std::string action = RecipeApp::CliUtils::getStringFromConsole("您想 'add' (添加) 还是 'remove' (移除) 菜谱? (add/remove): ");
                int recipeIdToManage = 0;

                if (action == "add" || action == "remove") {
                    std::string recipeIdStr = RecipeApp::CliUtils::getStringFromConsole("请输入要操作的菜谱ID: ");
                    try {
                        recipeIdToManage = std::stoi(recipeIdStr);
                        if (recipeIdToManage <= 0) {
                             spdlog::error("提供的菜谱ID '{}' 无效。", recipeIdToManage);
                             throw RecipeApp::Common::Exceptions::ValidationException("无效的菜谱ID: ID必须为正整数。");
                        }
                    } catch (const std::invalid_argument& ia) {
                        spdlog::error("输入的菜谱ID无效: '{}'. 错误: {}", recipeIdStr, ia.what());
                        throw RecipeApp::Common::Exceptions::ValidationException("菜谱ID必须是一个数字。");
                    } catch (const std::out_of_range& oor) {
                        spdlog::error("输入的菜谱ID数值过大: '{}'. 错误: {}", recipeIdStr, oor.what());
                        throw RecipeApp::Common::Exceptions::ValidationException("菜谱ID数值过大。");
                    }

                    // Verify recipe exists
                    if (!recipeManager_.findRecipeById(recipeIdToManage)) {
                        spdlog::warn("尝试操作但未找到ID为 {} 的菜谱。", recipeIdToManage);
                        std::cout << "错误: 未找到ID为 " << recipeIdToManage << " 的菜谱。" << std::endl;
                        return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
                    }

                } else {
                    std::cout << "无效操作。请输入 'add' 或 'remove'。" << std::endl;
                    return RecipeApp::Cli::EX_USAGE;
                }

                bool success = false;
                if (action == "add") {
                    // Check if already featured
                    bool alreadyFeatured = false;
                    for(int id : restaurantToManage.getFeaturedRecipeIds()){
                        if(id == recipeIdToManage) {
                            alreadyFeatured = true;
                            break;
                        }
                    }
                    if(alreadyFeatured){
                        std::cout << "菜谱 ID " << recipeIdToManage << " 已经存在于菜单中。" << std::endl;
                        return RecipeApp::Cli::EX_OK; // Or EX_DATAERR if considered an error
                    }
                    restaurantToManage.addFeaturedRecipe(recipeIdToManage); // Uses Restaurant domain method
                    success = restaurantManager_.updateRestaurant(restaurantToManage);
                    if (success) {
                        spdlog::info("菜谱 ID {} 已添加到餐馆 ID {} 的菜单。", recipeIdToManage, restaurantId);
                        std::cout << "菜谱 ID " << recipeIdToManage << " 已成功添加到菜单！" << std::endl;
                    }
                } else if (action == "remove") {
                     // Check if actually featured before trying to remove
                    bool isFeatured = false;
                    const auto& currentIds = restaurantToManage.getFeaturedRecipeIds();
                    if (std::find(currentIds.begin(), currentIds.end(), recipeIdToManage) != currentIds.end()) {
                        isFeatured = true;
                    }

                    if (!isFeatured) {
                        std::cout << "菜谱 ID " << recipeIdToManage << " 不在当前菜单中。" << std::endl;
                        return RecipeApp::Cli::EX_OK; // Or EX_DATAERR
                    }
                    restaurantToManage.removeFeaturedRecipe(recipeIdToManage); // Uses Restaurant domain method
                    success = restaurantManager_.updateRestaurant(restaurantToManage);
                    if (success) {
                        spdlog::info("菜谱 ID {} 已从餐馆 ID {} 的菜单中移除。", recipeIdToManage, restaurantId);
                        std::cout << "菜谱 ID " << recipeIdToManage << " 已成功从菜单中移除！" << std::endl;
                    }
                }

                if (success) {
                    return RecipeApp::Cli::EX_OK;
                } else {
                    spdlog::error("更新餐馆 ID {} 的菜单失败 (管理器返回false)。", restaurantId);
                    std::cout << "更新菜单失败。请检查日志。" << std::endl;
                    return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
                }

            } catch (const RecipeApp::Common::Exceptions::ValidationException& ve) {
                std::cout << "错误 (校验): " << ve.what() << std::endl;
                return RecipeApp::Cli::EX_DATAERR;
            } catch (const RecipeApp::Common::Exceptions::PersistenceException& pe) {
                std::cout << "错误 (持久化): " << pe.what() << std::endl;
                return RecipeApp::Cli::EX_IOERR;
            } catch (const std::exception& e) {
                spdlog::error("管理餐馆 ID {} 菜单时发生未知错误: {}", restaurantId, e.what());
                std::cout << "发生意外错误: " << e.what() << std::endl;
                return RecipeApp::Cli::EX_SOFTWARE;
            }
        }

        int RestaurantCommandHandler::handleSearchRestaurantsByName(const cxxopts::ParseResult& result) {
            spdlog::debug("处理按名称搜索餐馆命令。");
            if (!result.count("restaurant-search-name")) {
                 spdlog::error("按名称搜索餐馆命令 (--restaurant-search-name) 需要一个搜索词参数。");
                 throw RecipeApp::Common::Exceptions::ValidationException("按名称搜索餐馆需要提供搜索词。");
            }

            std::string searchTerm = result["restaurant-search-name"].as<std::string>();
            if (searchTerm.empty()) {
                spdlog::error("搜索词不能为空。");
                throw RecipeApp::Common::Exceptions::ValidationException("搜索词不能为空。");
            }

            bool partialMatch = result.count("partial-match") > 0; // Check if --partial-match flag is present

            std::cout << "--- 搜索餐馆 (名称: \"" << searchTerm << "\", 部分匹配: " << (partialMatch ? "是" : "否") << ") ---" << std::endl;

            try {
                std::vector<RecipeApp::Restaurant> restaurants = restaurantManager_.findRestaurantByName(searchTerm, partialMatch);
                if (restaurants.empty()) {
                    std::cout << "未找到名称匹配 '" << searchTerm << "' 的餐馆。" << std::endl;
                } else {
                    for (const auto& r : restaurants) {
                        std::cout << "ID: " << r.getId()
                                  << ", 名称: " << r.getName()
                                  << ", 地址: " << r.getAddress()
                                  << ", 联系方式: " << r.getContact()
                                  << std::endl;
                    }
                    std::cout << "共找到 " << restaurants.size() << " 个匹配的餐馆。" << std::endl;
                }
                return RecipeApp::Cli::EX_OK;
            } catch (const RecipeApp::Common::Exceptions::PersistenceException& pe) {
                spdlog::error("按名称搜索餐馆时发生持久化错误: {}", pe.what());
                std::cout << "错误: 搜索餐馆失败 - " << pe.what() << std::endl;
                return RecipeApp::Cli::EX_IOERR;
            } catch (const std::exception& e) {
                spdlog::error("按名称搜索餐馆时发生未知错误: {}", e.what());
                std::cout << "发生意外错误: " << e.what() << std::endl;
                return RecipeApp::Cli::EX_SOFTWARE;
            }
        }

        int RestaurantCommandHandler::handleSearchRestaurantsByCuisine(const cxxopts::ParseResult& result) {
            spdlog::debug("处理按菜系搜索餐馆命令。");
            if (!result.count("restaurant-search-cuisine")) {
                 spdlog::error("按菜系搜索餐馆命令 (--restaurant-search-cuisine) 需要一个菜系标签参数。");
                 throw RecipeApp::Common::Exceptions::ValidationException("按菜系搜索餐馆需要提供菜系标签。");
            }

            std::string cuisineTag = result["restaurant-search-cuisine"].as<std::string>();
            if (cuisineTag.empty()) {
                spdlog::error("菜系标签不能为空。");
                throw RecipeApp::Common::Exceptions::ValidationException("菜系标签不能为空。");
            }

            std::cout << "--- 搜索餐馆 (菜系: \"" << cuisineTag << "\") ---" << std::endl;

            try {
                std::vector<RecipeApp::Restaurant> restaurants = restaurantManager_.findRestaurantsByCuisine(cuisineTag, recipeManager_);
                if (restaurants.empty()) {
                    std::cout << "未找到供应菜系 '" << cuisineTag << "' 的餐馆。" << std::endl;
                } else {
                    for (const auto& r : restaurants) {
                        std::cout << "ID: " << r.getId()
                                  << ", 名称: " << r.getName()
                                  << ", 地址: " << r.getAddress()
                                  << ", 联系方式: " << r.getContact()
                                  << std::endl;
                    }
                    std::cout << "共找到 " << restaurants.size() << " 个匹配的餐馆。" << std::endl;
                }
                return RecipeApp::Cli::EX_OK;
            } catch (const RecipeApp::Common::Exceptions::PersistenceException& pe) {
                spdlog::error("按菜系搜索餐馆时发生持久化错误: {}", pe.what());
                std::cout << "错误: 搜索餐馆失败 - " << pe.what() << std::endl;
                return RecipeApp::Cli::EX_IOERR;
            } catch (const std::exception& e) {
                spdlog::error("按菜系搜索餐馆时发生未知错误: {}", e.what());
                std::cout << "发生意外错误: " << e.what() << std::endl;
                return RecipeApp::Cli::EX_SOFTWARE;
            }
        }
    } // namespace CliHandlers
} // namespace RecipeApp