#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "domain/user/User.h"
#include "domain/user/UserRepository.h" // Added UserRepository include
#include <string>
#include <optional> // For handling optional User from repository

// Forward declaration for User class if not fully included by UserRepository.h
// namespace RecipeApp { namespace Domain { namespace User { class User; } } }
// Forward declaration for UserRole enum if needed
// namespace RecipeApp { namespace Domain { namespace User { enum class UserRole; } } }

namespace RecipeApp
{
    // Forward declare User and UserRole if they are in a nested namespace and not pulled in by UserRepository.h
    // For example:
    // namespace Domain { namespace User { class User; enum class UserRole; } }
    // using Domain::User::User;
    // using Domain::User::UserRole;
    // However, User.h is already included, so direct use of User and UserRole should be fine.

    class UserManager
    {
    private:
        Domain::User::UserRepository &userRepository_; ///< Reference to the user repository
        const User *currentLoggedInUser;               ///< 指向当前登录用户的指针 (CLI 临时状态)

    public:
        /**
         * @brief 构造函数，接收一个 UserRepository 引用
         * @param userRepository 用户仓储的引用
         */
        explicit UserManager(Domain::User::UserRepository &userRepository);

        /**
         * @brief 默认析构函数
         */
        ~UserManager(); // Declaration only, definition will be in .cpp

        /**
         * @brief 注册新用户
         * @param username 用户名
         * @param password 密码
         * @return 指向新用户的指针，若用户名已存在则返回nullptr
         */
        User *registerUser(const std::string &username, const std::string &password);

        /**
         * @brief 管理员创建新用户（可指定角色）
         * @param username 新用户名
         * @param password 新用户密码
         * @param role 新用户角色
         * @param adminUser 执行操作的管理员用户对象
         * @return 指向新创建用户的指针，若操作失败（如权限不足、用户名已存在）则返回nullptr
         */
        User *createUserByAdmin(const std::string &username, const std::string &password, UserRole role, const User &adminUser);

        /**
         * @brief 用户登录
         * @param username 用户名
         * @param password 密码
         * @return 指向登录用户的常量指针，若凭据无效则返回nullptr。同时会更新内部的 currentLoggedInUser 状态。
         */
        const User *loginUser(const std::string &username, const std::string &password);

        /**
         * @brief 用户登出
         * 清除当前登录用户状态。
         */
        void logoutUser();

        /**
         * @brief 获取当前登录的用户
         * @return 指向当前登录用户的常量指针，如果未登录则返回 nullptr。
         */
        const User *getCurrentUser() const;

        /**
         * @brief 管理员删除用户
         * @param userId 待删除用户的ID
         * @param adminUser 执行操作的管理员用户对象
         * @return 是否删除成功（成功返回true，权限不足、用户不存在或试图删除自己则返回false）
         */
        bool deleteUser(int userId, const User &adminUser);

        /**
         * @brief 更新用户信息（包括管理员修改角色）
         * @param userToUpdate 包含更新信息的用户对象（其ID用于定位用户）
         * @param currentUser 执行此操作的用户对象（用于权限检查）
         * @return 是否更新成功
         */
        bool updateUser(const User &userToUpdate, const User &currentUser);

        /**
         * @brief 获取所有用户 (通过 UserRepository)
         * @return 包含所有用户的 std::vector
         */
        std::vector<User> getAllUsers() const;

        /**
         * @brief Adds a user from persistence (bypasses normal registration logic).
         * @param user The user object to add.
         */
        void addUserFromPersistence(const User &user);

        /**
         * @brief Sets the next user ID from persistence.
         * @param nextId The next ID to be used by the repository.
         */
        void setNextUserIdFromPersistence(int nextId);

        // Methods like getAllUsers, addUserDirectly, setNextUserId are removed
        // as their responsibilities are now handled by the UserRepository.
    };

} // namespace RecipeApp

#endif // USER_MANAGER_H