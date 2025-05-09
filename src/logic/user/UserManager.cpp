#include "logic/user/UserManager.h"
#include "domain/user/UserRepository.h"
#include "domain/user/User.h"
#include <iostream> // For std::cerr (optional, for logging)
#include <optional> // For std::optional
#include <vector>   // Required for userRepository_.findAll()

// Using declarations for convenience within this .cpp file
using RecipeApp::User;
using RecipeApp::UserRole;
using RecipeApp::Domain::User::UserRepository; // This one is likely correct as UserRepository is in Domain::User

namespace RecipeApp
{

    UserManager::UserManager(UserRepository &userRepository)
        : userRepository_(userRepository), currentLoggedInUser(nullptr)
    {
    }

    UserManager::~UserManager()
    {
        if (currentLoggedInUser)
        {
            delete currentLoggedInUser;
            currentLoggedInUser = nullptr;
        }
    }

    User *UserManager::registerUser(const std::string &username, const std::string &password)
    {
        if (userRepository_.findByUsername(username).has_value())
        {
            // std::cerr << "UserManager: Username '" << username << "' already exists." << std::endl;
            return nullptr;
        }

        // ID 0 or -1, repository assigns actual ID
        User newUser(0, username, password, UserRole::Normal);
        int newUserId = userRepository_.save(newUser);

        if (newUserId != -1)
        {
            // The plan requires returning a User*. This is problematic with value semantics
            // from the repository. The caller of this function will be responsible for
            // deleting the returned User object to prevent memory leaks.
            std::optional<User> savedUserOpt = userRepository_.findById(newUserId);
            if (savedUserOpt.has_value())
            {
                return new User(savedUserOpt.value()); // Caller must delete this.
            }
            // std::cerr << "UserManager: User registered (ID: " << newUserId << ") but could not be retrieved." << std::endl;
            return nullptr; // Should ideally not happen if save and findById are consistent.
        }
        // std::cerr << "UserManager: Failed to save new user '" << username << "' to repository." << std::endl;
        return nullptr;
    }

    const User *UserManager::loginUser(const std::string &username, const std::string &password)
    {
        if (currentLoggedInUser) // Clean up previous session's User object
        {
            delete currentLoggedInUser;
            currentLoggedInUser = nullptr;
        }

        std::optional<User> userOpt = userRepository_.findByUsername(username);
        if (userOpt.has_value())
        {
            if (userOpt->verifyPassword(password))
            {
                // Store a heap-allocated copy for the current session.
                // UserManager is responsible for deleting this in logoutUser or destructor.
                currentLoggedInUser = new User(userOpt.value());
                return currentLoggedInUser;
            }
            // else { std::cerr << "UserManager: Invalid password for user '" << username << "'." << std::endl; }
        }
        // else { std::cerr << "UserManager: User '" << username << "' not found." << std::endl; }
        return nullptr;
    }

    void UserManager::logoutUser()
    {
        if (currentLoggedInUser)
        {
            delete currentLoggedInUser;
            currentLoggedInUser = nullptr;
        }
    }

    const User *UserManager::getCurrentUser() const
    {
        return currentLoggedInUser;
    }

    bool UserManager::deleteUser(int userId, const User &adminUser)
    {
        if (adminUser.getRole() != UserRole::Admin)
        {
            // std::cerr << "UserManager: Permission denied. Only administrators can delete users." << std::endl;
            return false;
        }
        if (adminUser.getUserId() == userId)
        {
            // std::cerr << "UserManager: Administrator cannot delete themselves." << std::endl;
            return false;
        }

        std::optional<User> userToDeleteOpt = userRepository_.findById(userId);
        if (!userToDeleteOpt.has_value())
        {
            // std::cerr << "UserManager: User to delete (ID: " << userId << ") not found." << std::endl;
            return false;
        }

        // Prevent deleting the last admin
        if (userToDeleteOpt->getRole() == UserRole::Admin)
        {
            std::vector<User> allUsers = userRepository_.findAll();
            int adminCount = 0;
            for (const auto &u : allUsers)
            {
                if (u.getRole() == UserRole::Admin)
                {
                    adminCount++;
                }
            }
            if (adminCount <= 1)
            {
                // std::cerr << "UserManager: Cannot delete the last administrator." << std::endl;
                return false;
            }
        }

        // If the user being deleted is the currently logged-in user, log them out first.
        if (currentLoggedInUser && currentLoggedInUser->getUserId() == userId)
        {
            logoutUser(); // This will delete the currentLoggedInUser object
        }

        return userRepository_.remove(userId);
    }

    bool UserManager::updateUser(const User &userToUpdate, const User &currentUser)
    {
        std::optional<User> userInRepoOpt = userRepository_.findById(userToUpdate.getUserId());
        if (!userInRepoOpt.has_value())
        {
            // std::cerr << "UserManager: User to update (ID: " << userToUpdate.getUserId() << ") not found." << std::endl;
            return false;
        }
        User userInRepo = userInRepoOpt.value(); // Get a mutable copy of the user's current state.

        if (currentUser.getRole() == UserRole::Admin)
        {
            // Admin is trying to update a user.
            // Check for username collision if username is being changed.
            if (userToUpdate.getUsername() != userInRepo.getUsername())
            {
                std::optional<User> existingUserWithNewName = userRepository_.findByUsername(userToUpdate.getUsername());
                if (existingUserWithNewName.has_value() && existingUserWithNewName->getUserId() != userToUpdate.getUserId())
                {
                    // std::cerr << "UserManager: New username '" << userToUpdate.getUsername() << "' is already taken by another user." << std::endl;
                    return false;
                }
            }

            // Check if admin is trying to demote the last admin.
            if (userInRepo.getRole() == UserRole::Admin && userToUpdate.getRole() == UserRole::Normal)
            {
                // Ensure this is not the last admin.
                std::vector<User> allUsers = userRepository_.findAll();
                int adminCount = 0;
                for (const auto &u : allUsers)
                {
                    if (u.getRole() == UserRole::Admin)
                    {
                        adminCount++;
                    }
                }
                // If current userInRepo is an admin and is being demoted, and there's only 1 admin total.
                if (adminCount <= 1 && userInRepo.getUserId() == userToUpdate.getUserId())
                {
                    // std::cerr << "UserManager: Cannot demote the last administrator." << std::endl;
                    return false;
                }
            }

            // Admin can update. The userToUpdate object should have all correct fields.
            // If password was part of the update, User class should handle hashing.
            User finalUserToSave = userToUpdate; // Assume userToUpdate is correctly formed.
            // If password needs explicit handling (e.g. from plain text to hash):
            // if (userToUpdate.getPlainTextPassword() != userInRepo.getPlainTextPassword() && !userToUpdate.getPlainTextPassword().empty()) {
            //     finalUserToSave.setPassword(userToUpdate.getPlainTextPassword());
            // } else if (userToUpdate.getPasswordHash() != userInRepo.getPasswordHash() && !userToUpdate.getPasswordHash().empty()) {
            //     finalUserToSave.setPasswordHash(userToUpdate.getPasswordHash());
            // } else {
            //     finalUserToSave.setPasswordHash(userInRepo.getPasswordHash()); // Keep old hash if not changed
            // }

            bool success = userRepository_.save(finalUserToSave) != -1;
            if (success && currentLoggedInUser && currentLoggedInUser->getUserId() == finalUserToSave.getUserId())
            {
                // If the admin updated themselves, update the session user object.
                delete currentLoggedInUser;
                currentLoggedInUser = new User(finalUserToSave);
            }
            return success;
        }
        else if (currentUser.getRole() == UserRole::Normal)
        {
            // Normal user is trying to update their own info.
            if (currentUser.getUserId() != userToUpdate.getUserId())
            {
                // std::cerr << "UserManager: Normal user cannot update another user's information." << std::endl;
                return false; // Cannot update another user.
            }
            // Normal user can only update their password.
            // Username and role must remain unchanged.
            if (userToUpdate.getUsername() != userInRepo.getUsername() ||
                userToUpdate.getRole() != userInRepo.getRole())
            {
                // std::cerr << "UserManager: Normal user can only update their password. Username or role change detected." << std::endl;
                return false;
            }

            // Update password on the copy from repository, then save.
            userInRepo.setPassword(userToUpdate.getPlainTextPassword());

            bool success = userRepository_.save(userInRepo) != -1;
            if (success && currentLoggedInUser && currentLoggedInUser->getUserId() == userInRepo.getUserId())
            {
                // Update the session user object (specifically its password hash).
                delete currentLoggedInUser;
                currentLoggedInUser = new User(userInRepo);
            }
            return success;
        }
        return false; // Should not be reached if roles are handled.
    }

    User *UserManager::createUserByAdmin(const std::string &username, const std::string &password, UserRole role, const User &adminUser)
    {
        if (adminUser.getRole() != UserRole::Admin)
        {
            // std::cerr << "UserManager: Permission denied. Only administrators can create users." << std::endl;
            return nullptr;
        }
        if (userRepository_.findByUsername(username).has_value())
        {
            // std::cerr << "UserManager: Username '" << username << "' already exists (admin creation)." << std::endl;
            return nullptr;
        }

        User newUser(0, username, password, role); // ID 0 or -1, repository assigns
        int newUserId = userRepository_.save(newUser);

        if (newUserId != -1)
        {
            // Similar to registerUser, returning User* is problematic.
            // Caller is responsible for deleting this.
            std::optional<User> savedUserOpt = userRepository_.findById(newUserId);
            if (savedUserOpt.has_value())
            {
                return new User(savedUserOpt.value()); // Caller must delete this.
            }
            // std::cerr << "UserManager: Admin created user (ID: " << newUserId << ") but could not retrieve." << std::endl;
            return nullptr;
        }
        // std::cerr << "UserManager: Admin failed to save new user '" << username << "' to repository." << std::endl;
        return nullptr;
    }

} // namespace RecipeApp
// Add new method implementation at the end of the file, before the closing namespace brace.
// Assuming the namespace RecipeApp is already open.

std::vector<RecipeApp::User> RecipeApp::UserManager::getAllUsers() const
{
    return userRepository_.findAll();
}

// Make sure this is placed before the final '}' of the RecipeApp namespace,
// or adjust if the file structure is different.
// If the file ends with "}} // namespace RecipeApp::Logic" or similar,
// it should be placed before that.
// For now, appending to the end (line 0) should place it within the namespace
// if the file is structured typically.
// Add new method implementations at the end of the file, before the closing namespace brace.

void RecipeApp::UserManager::addUserFromPersistence(const User &user)
{
    userRepository_.save(user); // Assuming save handles both new and existing if ID is set
}

void RecipeApp::UserManager::setNextUserIdFromPersistence(int nextId)
{
    userRepository_.setNextId(nextId);
}