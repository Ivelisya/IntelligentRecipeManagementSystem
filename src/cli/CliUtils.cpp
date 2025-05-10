#include "CliUtils.h"

#include <algorithm>  // Required for std::all_of, std::find (used in search by parseCsvStringToVector, though not directly here)
#include <iostream>
#include <limits>     // Required for std::numeric_limits
#include <sstream>    // Required for std::stringstream (used for parsing tags)
#include <stdexcept>  // Required for std::exception
#include <string>
#include <utility>  // For std::pair
#include <vector>

// Note: domain/recipe/Recipe.h and domain/user/User.h are already included via
// CliUtils.h

namespace RecipeApp {
namespace CliUtils {
// Definition for verbose mode status
static bool verbose_mode = false;

void setVerbose(bool verbose) { verbose_mode = verbose; }

bool isVerbose() { return verbose_mode; }

std::string getPasswordFromConsole(const std::string &prompt) {
    std::string password;
    std::cout << prompt;
    // WARNING: Plain text password input - not secure for production
    std::getline(std::cin, password);  // 使用 getline 读取整行，允许空密码
    return password;
}

std::string getStringFromConsole(const std::string &prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

int getIntFromConsole(const std::string &prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        if (line.empty()) {
            std::cout << "输入不能为空，请输入一个整数。" << std::endl;
            continue;
        }
        try {
            size_t processed_chars;
            value = std::stoi(line, &processed_chars);
            if (processed_chars == line.length()) {
                break;
            } else {
                std::cout << "无效输入，请输入一个纯整数。" << std::endl;
            }
        } catch (const std::invalid_argument &ia) {
            (void)ia;  // Mark as intentionally unused
            std::cout << "无效输入，请输入一个整数。" << std::endl;
        } catch (const std::out_of_range &oor) {
            (void)oor;  // Mark as intentionally unused
            std::cout << "输入数字超出范围。" << std::endl;
        }
    }
    return value;
}

// RecipeApp::UserRole getRoleSelectionFromConsole()
// {
//     int choice = 0;
//     while (true)
//     {
//         std::cout << "请选择用户角色：" << std::endl;
//         std::cout << "1. 普通用户 (Normal)" << std::endl;
//         std::cout << "2. 管理员 (Admin)" << std::endl;
//         choice = getIntFromConsole("请输入选项 (1-2): ");
//         if (choice == 1 || choice == 2)
//             break;
//         std::cout << "无效选项，请重新输入。" << std::endl;
//     }
//     return (choice == 1) ? RecipeApp::UserRole::Normal :
//     RecipeApp::UserRole::Admin;
// }

// void displayUserDetailsBrief(const RecipeApp::User &user)
// {
//     std::cout << "  ID: " << user.getUserId()
//               << ", 用户名: " << user.getUsername()
//               << ", 角色: " << (user.getRole() == RecipeApp::UserRole::Admin
//               ? "管理员" : "普通用户")
//               << std::endl;
// }

RecipeApp::Difficulty getDifficultyFromConsole() {
    int choice = 0;
    while (true) {
        std::cout << "请选择难度级别：" << std::endl;
        std::cout << "1. 简单" << std::endl;
        std::cout << "2. 中等" << std::endl;
        std::cout << "3. 困难" << std::endl;
        choice = getIntFromConsole("请输入选项 (1-3): ");
        if (choice >= 1 && choice <= 3) {
            break;
        }
        std::cout << "无效选项，请重新输入。" << std::endl;
    }
    switch (choice) {
        case 1:
            return RecipeApp::Difficulty::Easy;
        case 2:
            return RecipeApp::Difficulty::Medium;
        case 3:
            return RecipeApp::Difficulty::Hard;
        default:
            // Should not reach here due to loop condition, but good practice
            // for switch
            std::cerr << "警告: getDifficultyFromConsole 到达了默认情况。"
                      << std::endl;
            return RecipeApp::Difficulty::Easy;
    }
}

std::vector<std::pair<std::string, std::string>> getIngredientsFromConsole() {
    std::vector<std::pair<std::string, std::string>> ingredients;
    std::cout << "请输入配料 (每行一个，格式：[配料名称] "
                 "[数量和单位]，例如：鸡蛋 2个。输入 'done' 或空行结束)："
              << std::endl;
    std::string line;
    while (true) {
        std::cout << "配料> ";
        std::getline(std::cin, line);
        if (line == "done" || line == "DONE" || line.empty()) {
            if (ingredients.empty() && line.empty()) {
                std::string confirmEnd = getStringFromConsole(
                    "未输入任何配料。确定要完成吗？ (y/n): ");
                if (confirmEnd != "y" && confirmEnd != "Y") continue;
            }
            break;
        }
        size_t lastSpace = line.find_last_of(" \t");
        std::string name, quantity;
        if (lastSpace != std::string::npos && lastSpace > 0 &&
            lastSpace < line.length() - 1) {
            name = line.substr(0, lastSpace);
            quantity = line.substr(lastSpace + 1);
        } else {
            name = line;
            quantity = "";  // Default to empty if no quantity part found
            std::cout << " (提示: 配料 '" << name << "' 未指定数量。数量将为空)"
                      << std::endl;
        }
        ingredients.push_back({name, quantity});
    }
    return ingredients;
}

std::vector<std::string> getStepsFromConsole() {
    std::vector<std::string> steps;
    std::cout << "请输入烹饪步骤 (每行一个步骤，输入 'done' 或空行结束)："
              << std::endl;
    std::string step_str;
    int stepNumber = 1;
    while (true) {
        std::cout << "步骤 " << stepNumber << ": ";
        std::getline(std::cin, step_str);
        if (step_str == "done" || step_str == "DONE" || step_str.empty()) {
            if (steps.empty() && step_str.empty()) {
                std::string confirmEnd = getStringFromConsole(
                    "未输入任何步骤。确定要完成吗？ (y/n): ");
                if (confirmEnd != "y" && confirmEnd != "Y") continue;
            }
            break;
        }
        steps.push_back(step_str);
        stepNumber++;
    }
    return steps;
}

std::vector<std::string> getTagsFromConsole(
    const std::vector<std::string> &currentTags) {
    std::vector<std::string> tags;
    bool clearing_current = false;
    if (!currentTags.empty()) {
        std::cout << "当前标签: ";
        for (size_t i = 0; i < currentTags.size(); ++i) {
            std::cout << currentTags[i]
                      << (i == currentTags.size() - 1 ? "" : ", ");
        }
        std::cout << std::endl;
        std::string keepOrClear = getStringFromConsole(
            "要修改当前标签吗? (y/n, 输入 'y' 清空并重新输入, 'n' 保留并追加, "
            "直接回车默认 'n'): ");
        if (keepOrClear == "y" || keepOrClear == "Y") {
            std::cout << "当前标签已清除，请重新输入所有标签。" << std::endl;
            clearing_current = true;
        } else {
            tags = currentTags;  // Keep current tags and append
        }
    }

    std::cout << "请输入标签 (每行一个，输入 'done' 或空行结束添加"
              << (clearing_current || currentTags.empty() ? "" : "新的")
              << "标签)：" << std::endl;
    std::string tag_str;
    int tagNumberDisplay = 1;  // For display prompt only

    while (true) {
        std::cout << "标签 " << tagNumberDisplay << ": ";
        std::getline(std::cin, tag_str);

        // Trim whitespace from tag_str
        size_t first = tag_str.find_first_not_of(" \t\n\r\f\v");
        if (std::string::npos == first) {  // Empty or all whitespace line
            if (tags.empty() &&
                clearing_current) {  // If clearing and first new tag is empty
                std::string confirmEnd = getStringFromConsole(
                    "未输入任何标签。确定要完成吗？ (y/n): ");
                if (confirmEnd != "y" && confirmEnd != "Y") {
                    continue;
                }
            } else if (tags.empty() && !clearing_current &&
                       currentTags.empty()) {  // No current tags, and first new
                                               // tag is empty
                std::string confirmEnd = getStringFromConsole(
                    "未输入任何标签。确定要完成吗？ (y/n): ");
                if (confirmEnd != "y" && confirmEnd != "Y") {
                    continue;
                }
            }
            break;
        }
        tag_str = tag_str.substr(
            first, (tag_str.find_last_not_of(" \t\n\r\f\v") - first + 1));

        if (tag_str == "done" || tag_str == "DONE") {
            break;
        }

        // Optional: check for duplicate tags before adding
        bool exists = false;
        for (const auto &existing_tag : tags) {
            if (existing_tag == tag_str) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            tags.push_back(tag_str);
        } else {
            std::cout << " (提示: 标签 '" << tag_str
                      << "' 已存在，不会重复添加)" << std::endl;
        }
        tagNumberDisplay++;
    }
    return tags;
}

std::vector<std::string> parseCsvStringToVector(const std::string &csv_string) {
    std::vector<std::string> result;
    if (csv_string.empty()) {  // Handle empty input string gracefully
        return result;
    }
    std::stringstream ss(csv_string);
    std::string item;
    while (std::getline(ss, item, ',')) {
        // Basic trim: remove leading and trailing whitespace
        size_t first = item.find_first_not_of(" \t\n\r\f\v");
        if (std::string::npos == first) {  // String is all whitespace
            continue;
        }
        size_t last = item.find_last_not_of(" \t\n\r\f\v");
        item = item.substr(first, (last - first + 1));
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    return result;
}

void displayRecipeDetailsBrief(const RecipeApp::Recipe &recipe) {
    // Cuisine is now part of tags, so it's not displayed directly here for
    // brevity. Tags could be displayed if desired, but brief usually means
    // minimal info.
    std::cout << "  ID: " << recipe.getRecipeId()
              << ", 名称: " << recipe.getName();
    // Optionally, display first few tags or a count
    if (!recipe.getTags().empty()) {
        std::cout << ", 标签: ";
        const auto &tags = recipe.getTags();
        for (size_t i = 0; i < tags.size() && i < 2;
             ++i) {  // Display max 2 tags for brief
            std::cout << tags[i] << (i < tags.size() - 1 && i < 1 ? ", " : "");
        }
        if (tags.size() > 2) {
            std::cout << "...";
        }
    }
    std::cout << std::endl;
}

void displayRecipeDetailsFull(const RecipeApp::Recipe &recipe) {
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "菜谱 ID: " << recipe.getRecipeId() << std::endl;
    std::cout << "名称: " << recipe.getName() << std::endl;
    // Cuisine is removed, tags will be displayed below
    std::cout << "烹饪时长: " << recipe.getCookingTime() << " 分钟"
              << std::endl;
    std::cout << "难度: ";
    switch (recipe.getDifficulty()) {
        case RecipeApp::Difficulty::Easy:
            std::cout << "简单";
            break;
        case RecipeApp::Difficulty::Medium:
            std::cout << "中等";
            break;
        case RecipeApp::Difficulty::Hard:
            std::cout << "困难";
            break;
    }
    std::cout << std::endl;

    std::cout << "配料:" << std::endl;
    if (recipe.getIngredients().empty()) {
        std::cout << "  (无配料信息)" << std::endl;
    } else {
        for (const auto &ing : recipe.getIngredients()) {
            std::cout << "  - " << ing.name << " (" << ing.quantity
                      << ")"  // Changed from .first and .second
                      << std::endl;
        }
    }

    std::cout << "步骤:" << std::endl;
    if (recipe.getSteps().empty()) {
        std::cout << "  (无步骤信息)" << std::endl;
    } else {
        int stepNum = 1;
        for (const auto &step : recipe.getSteps()) {
            std::cout << "  " << stepNum++ << ". " << step
                      << std::endl;  // Display step number
        }
    }

    // Display Tags
    std::cout << "标签:" << std::endl;
    if (recipe.getTags().empty()) {
        std::cout << "  (无标签信息)" << std::endl;
    } else {
        std::string tags_str;
        const auto &tags = recipe.getTags();
        for (size_t i = 0; i < tags.size(); ++i) {
            tags_str += tags[i];
            if (i < tags.size() - 1) {
                tags_str += ", ";
            }
        }
        std::cout << "  " << tags_str << std::endl;
    }

    if (recipe.getNutritionalInfo().has_value() &&
        !recipe.getNutritionalInfo().value().empty()) {
        std::cout << "营养信息: " << recipe.getNutritionalInfo().value()
                  << std::endl;
    }
    if (recipe.getImageUrl().has_value() &&
        !recipe.getImageUrl().value().empty()) {
        std::cout << "图片链接: " << recipe.getImageUrl().value() << std::endl;
    }
    std::cout << "----------------------------------------" << std::endl;
}

std::vector<int> parseCsvStringToIntVector(const std::string &csv_string) {
    std::vector<int> result;
    if (csv_string.empty()) {
        return result;
    }
    std::stringstream ss(csv_string);
    std::string item_str;
    int item_int;
    size_t processed_chars;

    while (std::getline(ss, item_str, ',')) {
        // Basic trim
        size_t first = item_str.find_first_not_of(" \t\n\r\f\v");
        if (std::string::npos == first) {
            continue; // Skip empty or all-whitespace segments
        }
        size_t last = item_str.find_last_not_of(" \t\n\r\f\v");
        item_str = item_str.substr(first, (last - first + 1));

        if (!item_str.empty()) {
            try {
                item_int = std::stoi(item_str, &processed_chars);
                // Check if the entire string was consumed by stoi
                if (processed_chars != item_str.length()) {
                    throw std::invalid_argument("CSV segment '" + item_str + "' contains non-numeric characters after number.");
                }
                result.push_back(item_int);
            } catch (const std::invalid_argument &ia) {
                // Re-throw with more context. The caller (CommandHandler) can catch this.
                throw std::invalid_argument("无法将CSV片段 '" + item_str + "' 解析为整数: " + ia.what());
            } catch (const std::out_of_range &oor) {
                // Re-throw with more context.
                throw std::out_of_range("CSV片段 '" + item_str + "' 中的数字超出范围: " + oor.what());
            }
        }
    }
    return result;
}

std::string difficultyToString(RecipeApp::Difficulty difficulty) {
    switch (difficulty) {
        case RecipeApp::Difficulty::Easy:
            return "简单";
        case RecipeApp::Difficulty::Medium:
            return "中等";
        case RecipeApp::Difficulty::Hard:
            return "困难";
        default:
            // It's good practice to log this unexpected case if spdlog is available here
            // spdlog::warn("Unknown RecipeApp::Difficulty value encountered: {}", static_cast<int>(difficulty));
            return "未知";
    }
}

}  // namespace CliUtils
}  // namespace RecipeApp