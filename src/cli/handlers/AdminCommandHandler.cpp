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
            const RecipeApp::User *currentUser = userManager.getCurrentUser();
            if (!currentUser)
            {
                std::cerr << "Error: Please login first." << std::endl;
                return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            }
            if (currentUser->getRole() != RecipeApp::UserRole::Admin)
            {
                std::cerr << "Error: Permission denied. Only administrators can view the user list." << std::endl;
                return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            }

            std::cout << "--- User List (Admin View) ---" << std::endl;
            const auto &allUsers = userManager.getAllUsers();
            if (allUsers.empty())
            {
                std::cout << "No users currently in the system." << std::endl;
                return RecipeApp::Cli::EX_OK; // Not an error
            }
            for (const auto &user : allUsers)
            {
                RecipeApp::CliUtils::displayUserDetailsBrief(user);
            }
            std::cout << "Total " << allUsers.size() << " users." << std::endl;
            return RecipeApp::Cli::EX_OK;
        }

        int AdminCommandHandler::handleAdminUserCreate(const cxxopts::ParseResult &result)
        {
            const RecipeApp::User *currentUser = userManager.getCurrentUser();
            if (!currentUser)
            {
                std::cerr << "Error: Please login first." << std::endl;
                return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            }
            if (currentUser->getRole() != RecipeApp::UserRole::Admin)
            {
                std::cerr << "Error: Permission denied. Only administrators can create users." << std::endl;
                return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            }

            std::cout << "--- Create New User (Admin) ---" << std::endl;
            std::string username = RecipeApp::CliUtils::getStringFromConsole("Enter new username: ");
            if (username.empty())
            {
                std::cerr << "Error: Username cannot be empty." << std::endl;
                return RecipeApp::Cli::EX_APP_INVALID_INPUT;
            }
            std::string password;
            std::string confirmPassword;
            while (true)
            {
                password = RecipeApp::CliUtils::getPasswordFromConsole("Enter new password: ");
                if (password.empty())
                {
                    std::cerr << "Error: Password cannot be empty. Please re-enter." << std::endl;
                    continue;
                }
                confirmPassword = RecipeApp::CliUtils::getPasswordFromConsole("Confirm new password: ");
                if (password == confirmPassword)
                {
                    break;
                }
                else
                {
                    std::cerr << "Error: Passwords do not match. Please re-enter." << std::endl;
                }
            }
            RecipeApp::UserRole role = RecipeApp::CliUtils::getRoleSelectionFromConsole();

            RecipeApp::User *newUser = userManager.createUserByAdmin(username, password, role, *currentUser);
            if (newUser)
            {
                std::cout << "User '" << username << "' created successfully (Role: "
                          << (role == RecipeApp::UserRole::Admin ? "Administrator" : "Normal User") << ")." << std::endl;
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "Failed to create user. Username may already exist." << std::endl;
                return RecipeApp::Cli::EX_APP_ALREADY_EXISTS; // Or EX_APP_OPERATION_FAILED
            }
        }

        int AdminCommandHandler::handleAdminUserUpdate(const cxxopts::ParseResult &result)
        {
            const RecipeApp::User *currentUser = userManager.getCurrentUser();
            if (!currentUser)
            {
                std::cerr << "Error: Please login first." << std::endl;
                return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            }
            if (currentUser->getRole() != RecipeApp::UserRole::Admin)
            {
                std::cerr << "Error: Permission denied. Only administrators can update user information." << std::endl;
                return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            }

            if (!result.count("admin-user-update"))
            {
                std::cerr << "Error: Missing argument (USER_ID) for admin-user-update command." << std::endl;
                std::cerr << "Usage: recipe-cli --admin-user-update <USER_ID>" << std::endl;
                return RecipeApp::Cli::EX_USAGE;
            }

            int userIdToUpdate = 0;
            try
            {
                userIdToUpdate = result["admin-user-update"].as<int>();
            }
            catch (const cxxopts::exceptions::exception &e)
            {
                std::cerr << "Error: Invalid USER_ID '" << result["admin-user-update"].as<std::string>() << "'. Please enter a number." << std::endl;
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
                std::cerr << "Error: User with ID " << userIdToUpdate << " not found." << std::endl;
                return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
            }

            std::cout << "--- Update User Information (ID: " << userIdToUpdate << ") ---" << std::endl;
            std::cout << "Current user information:" << std::endl;
            RecipeApp::CliUtils::displayUserDetailsBrief(originalUserCopy);
            std::cout << "Enter new user information (leave empty to keep current value for a field):" << std::endl;

            std::string newUsername = RecipeApp::CliUtils::getStringFromConsole("New username [" + originalUserCopy.getUsername() + "]: ");
            std::string newPassword = RecipeApp::CliUtils::getPasswordFromConsole("New password [******] (enter new password to change, leave empty to keep current): ");
            RecipeApp::UserRole newRole = originalUserCopy.getRole();
            std::cout << "Current role: " << (originalUserCopy.getRole() == RecipeApp::UserRole::Admin ? "Administrator" : "Normal User") << std::endl;
            std::string changeRoleChoice = RecipeApp::CliUtils::getStringFromConsole("Modify role? (y/n): ");
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

            if (userManager.updateUser(userWithUpdates, *currentUser))
            {
                std::cout << "User ID " << userIdToUpdate << " updated successfully!" << std::endl;
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "Failed to update user. Possible reasons: permission denied, new username conflict, attempt to remove last admin rights, etc." << std::endl;
                return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
            }
        }

        int AdminCommandHandler::handleAdminUserDelete(const cxxopts::ParseResult &result)
        {
            const RecipeApp::User *currentUser = userManager.getCurrentUser();
            if (!currentUser)
            {
                std::cerr << "Error: Please login first." << std::endl;
                return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            }
            if (currentUser->getRole() != RecipeApp::UserRole::Admin)
            {
                std::cerr << "Error: Permission denied. Only administrators can delete users." << std::endl;
                return RecipeApp::Cli::EX_APP_PERMISSION_DENIED;
            }

            if (!result.count("admin-user-delete"))
            {
                std::cerr << "Error: Missing argument (USER_ID) for admin-user-delete command." << std::endl;
                std::cerr << "Usage: recipe-cli --admin-user-delete <USER_ID>" << std::endl;
                return RecipeApp::Cli::EX_USAGE;
            }

            int userIdToDelete = 0;
            try
            {
                userIdToDelete = result["admin-user-delete"].as<int>();
            }
            catch (const cxxopts::exceptions::exception &e)
            {
                std::cerr << "Error: Invalid USER_ID '" << result["admin-user-delete"].as<std::string>() << "'. Please enter a number." << std::endl;
                return RecipeApp::Cli::EX_DATAERR;
            }

            if (currentUser->getUserId() == userIdToDelete)
            {
                std::cerr << "Error: Operation not allowed. Administrator cannot delete themselves." << std::endl;
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
                std::cerr << "Error: User with ID " << userIdToDelete << " not found." << std::endl;
                return RecipeApp::Cli::EX_APP_ITEM_NOT_FOUND;
            }

            std::cout << "Found user: " << usernameToDelete << " (ID: " << userIdToDelete << ")" << std::endl;
            std::string confirm = RecipeApp::CliUtils::getStringFromConsole("Are you sure you want to delete this user? (y/n): ");

            if (confirm == "y" || confirm == "Y")
            {
                if (userManager.deleteUser(userIdToDelete, *currentUser))
                {
                    std::cout << "User ID " << userIdToDelete << " deleted successfully!" << std::endl;
                    return RecipeApp::Cli::EX_OK;
                }
                else
                {
                    std::cerr << "Failed to delete user." << std::endl;
                    return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
                }
            }
            else
            {
                std::cout << "Delete operation cancelled." << std::endl;
                return RecipeApp::Cli::EX_OK; // User cancelled, not an error
            }
        }

    } // namespace CliHandlers
} // namespace RecipeApp