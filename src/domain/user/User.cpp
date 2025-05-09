#include "User.h"

namespace RecipeApp
{

    User::User(int id, const std::string &username, const std::string &password, UserRole role)
        : userId(id), username(username), role(role)
    {
        setPassword(password); // 初始化时设置密码
    }

    void User::setPassword(const std::string &password)
    {
        // 注意：明文存储密码存在安全风险，仅用于演示目的
        // 在实际应用中，应使用哈希算法（如 SHA-256）存储密码
        plainTextPassword = password;
    }

    bool User::verifyPassword(const std::string &password) const
    {
        // 由于是明文存储，直接比较
        // 在实际应用中，应比较哈希值
        return plainTextPassword == password;
    }

    UserRole User::getRole() const
    {
        return role;
    }

    void User::setRole(UserRole newRole)
    {
        // 注意：实际应用中应检查调用者的权限，确保只有管理员可以更改角色
        role = newRole;
    }

    void User::setUsername(const std::string &newUsername)
    {
        // 注意：实际应用中可能需要检查用户名是否已存在（例如通过 UserManager），
        // 或者在更高层级（如 UserManager::updateUser）进行此类检查。
        // User 类本身通常只负责更新其成员变量。
        // 另外，如果用户名是唯一标识符且用于查找，更改它可能需要更复杂的逻辑。
        // 此处假设 UserManager 会处理这些业务规则。
        this->username = newUsername;
    }

    // Removed redefinition of getHashedPassword() as it's defined inline in User.h

} // namespace RecipeApp