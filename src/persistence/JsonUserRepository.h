#ifndef JSON_USER_REPOSITORY_H
#define JSON_USER_REPOSITORY_H

#include "domain/user/UserRepository.h"
#include "domain/user/User.h"
#include "core/CustomLinkedList.h"
#include "json.hpp" // nlohmann::json
#include <string>
#include <fstream> // For file operations
#include <vector>  // To hold users temporarily during load/save

namespace RecipeApp
{
    namespace Persistence
    {

        using json = nlohmann::json;
        using CustomDataStructures::CustomLinkedList;
        using RecipeApp::User;
        using RecipeApp::Domain::User::UserRepository;

        class JsonUserRepository : public UserRepository
        {
        public:
            explicit JsonUserRepository(const std::string &filePath);

            // Load all users from the JSON file into memory
            bool load();
            // Save all users from memory back to the JSON file
            bool saveAll();

            // Implementation of UserRepository interface
            std::optional<User> findById(int userId) const override;
            std::optional<User> findByUsername(const std::string &username) const override;
            std::vector<User> findAll() const override; // Changed return type to match interface and implementation
            bool save(const User &user) override;       // Handles create/update in memory
            bool remove(int userId) override;
            void setNextId(int nextId) override;

            // Helper to get the next available user ID
            int getNextId() const; // This should also be override if in base

        private:
            std::string m_filePath;
            CustomLinkedList<User> m_users; // In-memory storage
            int m_nextId = 1;               // Simple counter for new IDs

            // JSON serialization/deserialization helpers (can be moved to a separate utility if needed)
            json userToJson(const User &user) const;
            User jsonToUser(const json &j) const;
        };

    } // namespace Persistence
} // namespace RecipeApp

#endif // JSON_USER_REPOSITORY_H