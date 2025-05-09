#ifndef USER_H
#define USER_H

#include <string>

namespace RecipeApp
{

    /**
     * @enum UserRole
     * @brief 用户角色枚举
     */
    enum class UserRole
    {
        Normal, ///< 普通用户
        Admin   ///< 管理员
    };

    /**
     * @class User
     * @brief 用户类，包含用户的基本信息和操作
     */
    class User
    {
    private:
        int userId;                    ///< 用户ID，唯一标识
        std::string username;          ///< 用户名，唯一
        std::string plainTextPassword; ///< 密码（明文存储，注释安全风险，应改为哈希存储）
        UserRole role;                 ///< 用户角色

    public:
        /**
         * @brief 构造函数
         * @param id 用户ID
         * @param username 用户名
         * @param password 密码
         * @param role 用户角色
         */
        User(int id, const std::string &username, const std::string &password, UserRole role = UserRole::Normal);

        /**
         * @brief 默认析构函数
         */
        ~User() = default;

        /**
         * @brief 设置密码（明文存储，存在安全风险）
         * @param password 新密码
         */
        void setPassword(const std::string &password);

        /**
         * @brief 验证密码
         * @param password 输入的密码
         * @return 密码是否正确
         */
        bool verifyPassword(const std::string &password) const;

        /**
         * @brief 获取用户角色
         * @return 用户角色
         */
        UserRole getRole() const;

        /**
         * @brief 设置用户角色（仅管理员可操作）
         * @param newRole 新角色
         */
        void setRole(UserRole newRole);

        /**
         * @brief 设置用户名
         * @param newUsername 新用户名
         */
        void setUsername(const std::string &newUsername);

        // Getter 方法
        int getUserId() const { return userId; }
        const std::string &getUsername() const { return username; }
        const std::string &getPlainTextPassword() const { return plainTextPassword; } // Renamed for clarity, for persistence

        // Overload operator== for CustomLinkedList operations
        bool operator==(const User &other) const
        {
            return this->userId == other.userId;
        }
    };

} // namespace RecipeApp

#endif // USER_H