#include "UserCommandHandler.h"
#include "cli/CliUtils.h"     // For helper functions like getStringFromConsole, etc.
#include "cli/ExitCodes.h"    // For standardized exit codes
#include "domain/user/User.h" // For UserRole comparison
#include <iostream>
#include <string>
#include <vector> // Include vector for completeness if needed, though not directly used here

namespace RecipeApp
{
    namespace CliHandlers
    {

        // Updated constructor to only take UserManager
        UserCommandHandler::UserCommandHandler(UserManager &userManager)
            : m_userManager(userManager) {}

        int UserCommandHandler::handleLogin(const cxxopts::ParseResult &result)
        {
            std::string username;
            std::string password;

            if (result.count("login") && !result["login"].as<std::string>().empty())
            {
                username = result["login"].as<std::string>();
                if (CliUtils::isVerbose())
                {
                    std::cout << "[Debug] Username from argument: " << username << std::endl;
                }
            }
            else
            {
                username = CliUtils::getStringFromConsole("Enter username: ");
            }
            if (username.empty())
            {
                std::cerr << "Error: Username cannot be empty." << std::endl;
                return RecipeApp::Cli::EX_APP_INVALID_INPUT;
            }
            password = CliUtils::getPasswordFromConsole("Enter password: ");
            if (password.empty())
            {
                std::cerr << "Error: Password cannot be empty." << std::endl;
                return RecipeApp::Cli::EX_APP_INVALID_INPUT;
            }

            const User *user = m_userManager.loginUser(username, password);
            if (user)
            {
                std::cout << "Login successful! Welcome, " << user->getUsername() << "!" << std::endl;
                std::cout << "Your role is: " << (user->getRole() == UserRole::Admin ? "Administrator" : "Normal User") << std::endl;
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "Login failed. Incorrect username or password." << std::endl;
                return RecipeApp::Cli::EX_APP_LOGIN_FAILED;
            }
        }

        int UserCommandHandler::handleRegister(const cxxopts::ParseResult &result)
        {
            std::string username;
            std::string password;
            std::string confirmPassword;

            if (result.count("register") && !result["register"].as<std::string>().empty())
            {
                username = result["register"].as<std::string>();
                if (CliUtils::isVerbose())
                {
                    std::cout << "[Debug] Register username from argument: " << username << std::endl;
                }
            }
            else
            {
                username = CliUtils::getStringFromConsole("Enter new username: ");
            }
            if (username.empty())
            {
                std::cerr << "Error: Username cannot be empty." << std::endl;
                return RecipeApp::Cli::EX_APP_INVALID_INPUT;
            }

            while (true)
            {
                password = CliUtils::getPasswordFromConsole("Enter new password: ");
                if (password.empty())
                {
                    std::cerr << "Error: Password cannot be empty. Please re-enter." << std::endl;
                    // No specific exit code here, loop continues
                    continue;
                }
                confirmPassword = CliUtils::getPasswordFromConsole("Confirm new password: ");
                if (password == confirmPassword)
                {
                    break;
                }
                else
                {
                    std::cerr << "Error: Passwords do not match. Please re-enter." << std::endl;
                }
            }

            User *newUser = m_userManager.registerUser(username, password);
            if (newUser)
            {
                // IMPORTANT: newUser pointer from registerUser might be unsafe or null depending on implementation.
                // The success of registerUser implies the user exists in the repository.
                // We should avoid using the returned pointer directly if possible.
                // Let's adjust the success message.
                std::cout << "Registration successful! User '" << username << "' created. Please use 'login' command to log in." << std::endl;
                // Removed call to m_persistenceManager.saveData()
                // The repository handles saving internally.
                // We need to free the memory if registerUser returned a heap-allocated object.
                delete newUser; // Assuming registerUser returns a new User* that needs deletion.
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "Registration failed. Username may already exist." << std::endl;
                return RecipeApp::Cli::EX_APP_ALREADY_EXISTS;
            }
        }

        int UserCommandHandler::handleLogout(const cxxopts::ParseResult &result)
        {
            if (m_userManager.getCurrentUser())
            {
                m_userManager.logoutUser();
                std::cout << "Successfully logged out." << std::endl;
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cout << "No user currently logged in." << std::endl;
                return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN; // Or EX_OK if not considered an error
            }
        }

        int UserCommandHandler::handleUserProfile(const cxxopts::ParseResult &result)
        {
            const User *currentUser = m_userManager.getCurrentUser();
            if (currentUser)
            {
                std::cout << "--- Current User Information ---" << std::endl;
                std::cout << "User ID: " << currentUser->getUserId() << std::endl;
                std::cout << "Username: " << currentUser->getUsername() << std::endl;
                std::cout << "Role: " << (currentUser->getRole() == UserRole::Admin ? "Administrator" : "Normal User") << std::endl;
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "Error: Please login first to view user profile." << std::endl;
                return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            }
        }

        int UserCommandHandler::handleUpdateProfile(const cxxopts::ParseResult &result)
        {
            const User *currentUser = m_userManager.getCurrentUser();
            if (!currentUser)
            {
                std::cerr << "Error: Please login first to update profile." << std::endl;
                return RecipeApp::Cli::EX_APP_NOT_LOGGED_IN;
            }

            std::cout << "--- Update Profile (Password) ---" << std::endl;
            std::string currentPassword = CliUtils::getPasswordFromConsole("Enter current password for verification: ");
            if (!currentUser->verifyPassword(currentPassword))
            {
                std::cerr << "Error: Current password verification failed." << std::endl;
                return RecipeApp::Cli::EX_APP_PERMISSION_DENIED; // Or a specific password error
            }

            std::string newPassword;
            std::string confirmPassword;
            while (true)
            {
                newPassword = CliUtils::getPasswordFromConsole("Enter new password: ");
                if (newPassword.empty())
                {
                    std::cerr << "Error: New password cannot be empty. Please re-enter." << std::endl;
                    continue;
                }
                confirmPassword = CliUtils::getPasswordFromConsole("Confirm new password: ");
                if (newPassword == confirmPassword)
                {
                    break;
                }
                else
                {
                    std::cerr << "Error: New passwords do not match. Please re-enter." << std::endl;
                }
            }

            User userWithUpdates = *currentUser;
            userWithUpdates.setPassword(newPassword);

            if (m_userManager.updateUser(userWithUpdates, *currentUser))
            {
                std::cout << "Password updated successfully!" << std::endl;
                // Removed call to m_persistenceManager.saveData()
                // The repository handles saving internally via UserManager::updateUser.
                return RecipeApp::Cli::EX_OK;
            }
            else
            {
                std::cerr << "Password update failed. An internal error occurred." << std::endl;
                return RecipeApp::Cli::EX_APP_OPERATION_FAILED;
            }
        }

    } // namespace CliHandlers
} // namespace RecipeApp