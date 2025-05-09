#include "cli/handlers/AdminCommandHandler.h"
#include "cli/CliUtils.h"
#include "cli/ExitCodes.h"         // For standardized exit codes
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

        AdminCommandHandler::AdminCommandHandler(RecipeApp::UserManager &um)
            : userManager(um) {}

        int AdminCommandHandler::handleAdminUserList(const cxxopts::ParseResult &result)
        {
            // const RecipeApp::User *currentUser = userManager.getCurrentUser(); // No longer needed for auth check
            // if (!currentUser) // This check is no longer needed
            // {
            //     std::cerr << "Error: Please login first." << std::endl;
            //     return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            // }
            // if (currentUser->getRole() != RecipeApp::UserRole::Admin) // This check is no longer needed
            // {
            //     std::cerr << "Error: Permission denied. Only administrators can view the user list." << std::endl;
            //     return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            // }

            std::cout << "--- User List ---" << std::endl; // Simplified title
            const auto &allUsers = userManager.getAllUsers();
            if (allUsers.empty())
            {
                std::cout << "系统中当前没有用户。" << std::endl;
                return RecipeApp::Cli::EX_OK; // Not an error
            }
            for (const auto &user : allUsers)
            {
                RecipeApp::CliUtils::displayUserDetailsBrief(user);
            }
            std::cout << "共 " << allUsers.size() << " 个用户。" << std::endl;
            return RecipeApp::Cli::EX_OK;
        }

        int AdminCommandHandler::handleAdminUserCreate(const cxxopts::ParseResult &result)
        {
            // const RecipeApp::User *currentUser = userManager.getCurrentUser(); // No longer needed for auth check
            // if (!currentUser) // This check is no longer needed
            // {
            //     std::cerr << "Error: Please login first." << std::endl;
            //     return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            // }
            // if (currentUser->getRole() != RecipeApp::UserRole::Admin) // This check is no longer needed
            // {
            //     std::cerr << "Error: Permission denied. Only administrators can create users." << std::endl;
            //     return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            // }

            std::cout << "--- 创建新用户 ---" << std::endl; // Simplified title
            std::string username = RecipeApp::CliUtils::getStringFromConsole("请输入新用户名: ");
            if (username.empty())
            {
                std::cerr << "错误：用户名不能为空。" << std::endl;
                return RecipeApp::Cli::EX_APP_INVALID_INPUT;
            }
            std::string password;
            std::string confirmPassword;
            while (true)
            {
                password = RecipeApp::CliUtils::getPasswordFromConsole("请输入新密码: ");
                if (password.empty())
                {
                    std::cerr << "错误：密码不能为空，请重新输入。" << std::endl;
                    continue;
                }
                confirmPassword = RecipeApp::CliUtils::getPasswordFromConsole("请确认新密码: ");
                if (password == confirmPassword)
                {
                    break;
                }
                else
                {
                    std::cerr << "错误：两次输入的密码不匹配，请重新输入。" << std::endl;
                }
            }
            RecipeApp::UserRole role = RecipeApp::CliUtils::getRoleSelectionFromConsole();

            // Pass the default admin user from userManager, or adjust createUserByAdmin signature
            const RecipeApp::User *adminUser = userManager.getCurrentUser(); // This will be the default admin
            RecipeApp::User *newUser = userManager.createUserByAdmin(username, password, role, *adminUser);
            if (newUser)
            {
                // Since the created user object by createUserByAdmin is heap allocated and needs deletion by caller
                // and we don't use it further here, we should delete it.
                // However, the original design of createUserByAdmin in UserManager might return a pointer
                // that the UserManager itself doesn't own or that is a copy of what's in the repository.
                // Given the changes, createUserByAdmin in UserManager now returns a new User* that must be deleted.
                std::cout << "用户 '" << username << "' 创建成功 (角色: "
                          << (role == RecipeApp::UserRole::Admin ? "管理员" : "普通用户") << ")。" << std::endl;
                delete newUser; // Clean up the returned User object as it's not used further here.
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "创建用户失败。用户名可能已存在。" << std::endl;
                return RecipeApp::Cli::EX_APP_ALREADY_EXISTS; // Or EX_APP_OPERATION_FAILED
            }
        }

        int AdminCommandHandler::handleAdminUserUpdate(const cxxopts::ParseResult &result)
        {
            // const RecipeApp::User *currentUser = userManager.getCurrentUser(); // No longer needed for auth check
            // if (!currentUser) // This check is no longer needed
            // {
            //     std::cerr << "Error: Please login first." << std::endl;
            //     return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            // }
            // if (currentUser->getRole() != RecipeApp::UserRole::Admin) // This check is no longer needed
            // {
            //     std::cerr << "Error: Permission denied. Only administrators can update user information." << std::endl;
            //     return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            // }

            if (!result.count("admin-user-update"))
            {
                std::cerr << "错误：admin-user-update 命令缺少参数 (USER_ID)。" << std::endl;
                std::cerr << "用法: recipe-cli --admin-user-update <用户ID>" << std::endl;
                return RecipeApp::Cli::EX_USAGE;
            }

            int userIdToUpdate = 0;
            try
            {
                userIdToUpdate = result["admin-user-update"].as<int>();
            }
            catch (const cxxopts::exceptions::exception &e)
            {
                std::cerr << "错误：无效的用户ID '" << result["admin-user-update"].as<std::string>() << "'。请输入一个数字。" << std::endl;
                return RecipeApp::Cli::EX_DATAERR;
            }

            bool found = false;
            RecipeApp::User originalUserCopy(0, "", "");
            for (const auto &user : userManager.getAllUsers())
            {
                if (user.getUserId() == userIdToUpdate)
                {
                    originalUserCopy = user;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                std::cerr << "错误：未找到ID为 " << userIdToUpdate << " 的用户。" << std::endl;
                return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
            }

            std::cout << "--- 更新用户信息 (ID: " << userIdToUpdate << ") ---" << std::endl;
            std::cout << "当前用户信息：" << std::endl;
            RecipeApp::CliUtils::displayUserDetailsBrief(originalUserCopy);
            std::cout << "请输入新的用户信息 (留空则表示保留当前值)：" << std::endl;

            std::string newUsername = RecipeApp::CliUtils::getStringFromConsole("新用户名 [" + originalUserCopy.getUsername() + "]: ");
            std::string newPassword = RecipeApp::CliUtils::getPasswordFromConsole("新密码 [******] (输入新密码以更改，留空则保持不变): ");
            RecipeApp::UserRole newRole = originalUserCopy.getRole();
            std::cout << "当前角色: " << (originalUserCopy.getRole() == RecipeApp::UserRole::Admin ? "管理员" : "普通用户") << std::endl;
            std::string changeRoleChoice = RecipeApp::CliUtils::getStringFromConsole("修改角色? (y/n): ");
            if (changeRoleChoice == "y" || changeRoleChoice == "Y")
            {
                newRole = RecipeApp::CliUtils::getRoleSelectionFromConsole();
            }

            RecipeApp::User userWithUpdates = originalUserCopy;
            if (!newUsername.empty())
                userWithUpdates.setUsername(newUsername);
            if (!newPassword.empty())
                userWithUpdates.setPassword(newPassword);
            userWithUpdates.setRole(newRole);

            const RecipeApp::User *adminUser = userManager.getCurrentUser(); // This will be the default admin
            if (userManager.updateUser(userWithUpdates, *adminUser))
            {
                std::cout << "用户 ID " << userIdToUpdate << " 更新成功！" << std::endl;
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "更新用户失败。可能原因：权限不足、新用户名冲突、试图移除最后一个管理员权限等。" << std::endl;
                return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
            }
        }

        int AdminCommandHandler::handleAdminUserDelete(const cxxopts::ParseResult &result)
        {
            // const RecipeApp::User *currentUser = userManager.getCurrentUser(); // No longer needed for auth check
            // if (!currentUser) // This check is no longer needed
            // {
            //     std::cerr << "Error: Please login first." << std::endl;
            //     return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            // }
            // if (currentUser->getRole() != RecipeApp::UserRole::Admin) // This check is no longer needed
            // {
            //     std::cerr << "Error: Permission denied. Only administrators can delete users." << std::endl;
            //     return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            // }

            if (!result.count("admin-user-delete"))
            {
                std::cerr << "错误：admin-user-delete 命令缺少参数 (USER_ID)。" << std::endl;
                std::cerr << "用法: recipe-cli --admin-user-delete <用户ID>" << std::endl;
                return RecipeApp::Cli::EX_USAGE;
            }

            int userIdToDelete = 0;
            try
            {
                userIdToDelete = result["admin-user-delete"].as<int>();
            }
            catch (const cxxopts::exceptions::exception &e)
            {
                std::cerr << "错误：无效的用户ID '" << result["admin-user-delete"].as<std::string>() << "'。请输入一个数字。" << std::endl;
                return RecipeApp::Cli::EX_DATAERR;
            }

            const RecipeApp::User *adminUser = userManager.getCurrentUser();     // This will be the default admin
            if (adminUser->getUserId() == userIdToDelete && userIdToDelete == 0) // Check against default admin's ID (0)
            {
                std::cerr << "错误：操作不允许。无法删除默认管理员用户。" << std::endl;
                return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            }

            bool found = false;
            std::string usernameToDelete;
            for (const auto &user : userManager.getAllUsers())
            {
                if (user.getUserId() == userIdToDelete)
                {
                    usernameToDelete = user.getUsername();
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                std::cerr << "错误：未找到ID为 " << userIdToDelete << " 的用户。" << std::endl;
                return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
            }

            std::cout << "找到用户: " << usernameToDelete << " (ID: " << userIdToDelete << ")" << std::endl;
            std::string confirm = RecipeApp::CliUtils::getStringFromConsole("您确定要删除这个用户吗？ (y/n): ");

            if (confirm == "y" || confirm == "Y")
            {
                const RecipeApp::User *adminUserForDelete = userManager.getCurrentUser(); // This will be the default admin
                if (userManager.deleteUser(userIdToDelete, *adminUserForDelete))
                {
                    std::cout << "用户 ID " << userIdToDelete << " 删除成功！" << std::endl;
                    return RecipeApp::Cli::EX_OK;
                }
                else
                {
                    std::cerr << "删除用户失败。" << std::endl;
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