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
        : userRepository_(userRepository),
          m_defaultAdminUser(0, "admin", "admin", UserRole::Admin) // Initialize default admin
    {
        // No session loading needed. User is always the default admin.
    }

    UserManager::~UserManager()
    {
        // m_defaultAdminUser is a member object, no explicit delete needed for it.
    }

    const User *UserManager::getCurrentUser() const
    {
        return &m_defaultAdminUser;
    }

    bool UserManager::deleteUser(int userId, const User & /*adminUser*/)
    {
        // Current user is always admin.
        // Prevent deleting the conceptual default admin if it has ID 0.
        if (m_defaultAdminUser.getUserId() == userId && userId == 0)
        {
            return false;
        }

        std::optional<User> userToDeleteOpt = userRepository_.findById(userId);
        if (!userToDeleteOpt.has_value())
        {
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
                return false;
            }
        }

        return userRepository_.remove(userId);
    }

    bool UserManager::updateUser(const User &userToUpdate, const User & /*currentUser*/)
    {
        // Current user is always admin.
        std::optional<User> userInRepoOpt = userRepository_.findById(userToUpdate.getUserId());
        if (!userInRepoOpt.has_value())
        {
            return false;
        }
        User userInRepo = userInRepoOpt.value(); // Get a mutable copy of the user's current state.

        // Check for username collision if username is being changed.
        if (userToUpdate.getUsername() != userInRepo.getUsername())
        {
            std::optional<User> existingUserWithNewName = userRepository_.findByUsername(userToUpdate.getUsername());
            if (existingUserWithNewName.has_value() && existingUserWithNewName->getUserId() != userToUpdate.getUserId())
            {
                return false;
            }
        }

        // Check if admin is trying to demote the last *persisted* admin.
        if (userInRepo.getRole() == UserRole::Admin && userToUpdate.getRole() == UserRole::Normal)
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
            if (adminCount <= 1 && userInRepo.getUserId() == userToUpdate.getUserId())
            {
                return false;
            }
        }

        User finalUserToSave = userToUpdate;
        bool success = userRepository_.save(finalUserToSave) != -1;
        return success;
    }

    User *UserManager::createUserByAdmin(const std::string &username, const std::string &password, UserRole role, const User & /*adminUser*/)
    {
        // Current user is always admin.
        if (userRepository_.findByUsername(username).has_value())
        {
            return nullptr;
        }

        User newUser(0, username, password, role); // ID 0 or -1, repository assigns
        int newUserId = userRepository_.save(newUser);

        if (newUserId != -1)
        {
            std::optional<User> savedUserOpt = userRepository_.findById(newUserId);
            if (savedUserOpt.has_value())
            {
                return new User(savedUserOpt.value()); // Caller must delete this.
            }
            return nullptr;
        }
        return nullptr;
    }

} // namespace RecipeApp

std::vector<RecipeApp::User> RecipeApp::UserManager::getAllUsers() const
{
    return userRepository_.findAll();
}

void RecipeApp::UserManager::addUserFromPersistence(const User &user)
{
    userRepository_.save(user); // Assuming save handles both new and existing if ID is set
}

void RecipeApp::UserManager::setNextUserIdFromPersistence(int nextId)
{
    userRepository_.setNextId(nextId);
}