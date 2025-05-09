#ifndef PERSISTENCE_MANAGER_H
#define PERSISTENCE_MANAGER_H

#include "logic/user/UserManager.h"             // Updated path
#include "logic/recipe/RecipeManager.h"         // Updated path
#include "logic/restaurant/RestaurantManager.h" // Updated path
#include <string>

namespace RecipeApp
{

    /**
     * @class PersistenceManager
     * @brief 数据持久化管理类，负责将数据保存到文件和从文件加载数据
     */
    class PersistenceManager
    {
    private:
        std::string userFilePath;       ///< 用户数据文件路径
        std::string recipeFilePath;     ///< 食谱数据文件路径
        std::string restaurantFilePath; ///< 餐厅数据文件路径 (Added)

    public:
        /**
         * @brief 构造函数
         * @param userPath 用户数据文件路径
         * @param recipePath 食谱数据文件路径
         * @param restaurantPath 餐厅数据文件路径 (Added)
         */
        PersistenceManager(const std::string &userPath, const std::string &recipePath, const std::string &restaurantPath); // Modified

        /**
         * @brief 默认析构函数
         */
        ~PersistenceManager() = default;

        /**
         * @brief 保存数据到文件
         * @param userManager 用户管理对象
         * @param recipeManager 食谱管理对象
         * @param restaurantManager 餐厅管理对象 (Added)
         * @return 是否保存成功
         */
        bool saveData(const UserManager &userManager, const RecipeManager &recipeManager, const RestaurantManager &restaurantManager) const; // Modified

        /**
         * @brief 从文件加载数据
         * @param userManager 用户管理对象
         * @param recipeManager 食谱管理对象
         * @param restaurantManager 餐厅管理对象 (Added)
         * @return 是否加载成功
         */
        bool loadData(UserManager &userManager, RecipeManager &recipeManager, RestaurantManager &restaurantManager) const; // Modified
    };

} // namespace RecipeApp

#endif // PERSISTENCE_MANAGER_H