#ifndef CLI_UTILS_H
#define CLI_UTILS_H

#include <string>
#include <utility>  // For std::pair
#include <vector>

#include "domain/recipe/Recipe.h"  // For RecipeApp::Difficulty
// #include "domain/user/User.h"     // User domain object removed

namespace RecipeApp {
namespace CliUtils {
// Verbose mode functions
void setVerbose(bool verbose);
bool isVerbose();

// 从控制台获取字符串输入
std::string getStringFromConsole(const std::string &prompt);

// 从控制台获取整数输入
int getIntFromConsole(const std::string &prompt);

// 从控制台获取密码输入 (注意：简单实现，生产环境需更安全)
std::string getPasswordFromConsole(const std::string &prompt);

// 从控制台获取菜谱难度选择
RecipeApp::Difficulty getDifficultyFromConsole();

// 从控制台获取食材列表
std::vector<std::pair<std::string, std::string>> getIngredientsFromConsole();

// 从控制台获取制作步骤
std::vector<std::string> getStepsFromConsole();

// 从控制台获取标签列表 (逐条输入)
std::vector<std::string> getTagsFromConsole(
    const std::vector<std::string> &currentTags = {});

// 解析逗号分隔的字符串到vector
std::vector<std::string> parseCsvStringToVector(const std::string &csv_string);

// 从控制台获取用户角色选择
// RecipeApp::UserRole getRoleSelectionFromConsole(); // User related function
// removed

// 简要显示用户详情
// void displayUserDetailsBrief(const RecipeApp::User &user); // User related
// function removed

// 简要显示菜谱详情
void displayRecipeDetailsBrief(const RecipeApp::Recipe &recipe);

// 完整显示菜谱详情
void displayRecipeDetailsFull(const RecipeApp::Recipe &recipe);

}  // namespace CliUtils
}  // namespace RecipeApp

#endif  // CLI_UTILS_H