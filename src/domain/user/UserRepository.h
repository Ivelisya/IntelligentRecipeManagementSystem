#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include "domain/user/User.h"
#include "core/CustomLinkedList.h"
#include <string>
#include <optional>
#include <vector> // Added for std::vector

namespace RecipeApp
{
    namespace Domain
    {
        namespace User
        {
            class UserRepository
            {
            public:
                virtual ~UserRepository() = default;

                virtual std::optional<RecipeApp::User> findById(int userId) const = 0;
                virtual std::optional<RecipeApp::User> findByUsername(const std::string &username) const = 0;
                virtual std::vector<RecipeApp::User> findAll() const = 0; // Changed return type
                virtual bool save(const RecipeApp::User &user) = 0;       // Handles both create and update
                virtual bool remove(int userId) = 0;

                /**
                 * @brief Sets the next available ID for a new user (for persistence loading).
                 * @param nextId The next ID to be used.
                 */
                virtual void setNextId(int nextId) = 0;
                // Might need a way to get next ID or let database handle it.
                // For now, UserManager can handle ID generation if saving a new user.
            };
        } // namespace User
    } // namespace Domain
} // namespace RecipeApp

#endif // USER_REPOSITORY_H