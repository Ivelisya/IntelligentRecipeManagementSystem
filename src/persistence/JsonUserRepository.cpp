#include "persistence/JsonUserRepository.h"
#include "domain/user/User.h" // Ensure User class definition is included
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm> // For std::find_if, std::remove_if

namespace RecipeApp
{
    namespace Persistence
    {

        // Constructor
        JsonUserRepository::JsonUserRepository(const std::string &filePath)
            : m_filePath(filePath), m_nextId(1) // Initialize nextId
        {
            // Load data immediately upon construction? Or require explicit load call?
            // Let's require explicit load for now for better control.
        }

        // --- JSON Helper Functions ---

        json JsonUserRepository::userToJson(const User &user) const
        {
            json j;
            j["id"] = user.getUserId();
            j["username"] = user.getUsername();
            // IMPORTANT: Storing plain text password - highly insecure for production!
            // In a real app, store a hashed password.
            j["password"] = user.getPlainTextPassword();
            j["role"] = (user.getRole() == RecipeApp::UserRole::Admin) ? "Admin" : "Normal"; // Corrected namespace
            return j;
        }

        User JsonUserRepository::jsonToUser(const json &j) const
        {
            int id = j.value("id", 0);
            std::string username = j.value("username", "");
            std::string password = j.value("password", ""); // Plain text password from JSON
            std::string roleStr = j.value("role", "Normal");
            RecipeApp::UserRole role = (roleStr == "Admin") ? RecipeApp::UserRole::Admin : RecipeApp::UserRole::Normal; // Corrected namespace

            // Assuming User constructor takes plain text password for now
            // Need to include User.h for the constructor call, already done via JsonUserRepository.h -> UserRepository.h -> User.h
            return User(id, username, password, role);
        }

        // --- Load/Save Operations ---

        bool JsonUserRepository::load()
        {
            std::ifstream file(m_filePath);
            if (!file.is_open())
            {
                // If the file doesn't exist, it's not necessarily an error on first run.
                // We can start with an empty user list.
                std::cerr << "Warning: Could not open user data file for reading: " << m_filePath << ". Starting with an empty user list." << std::endl;
                m_users.clearList(); // Use clearList()
                m_nextId = 1;        // Reset ID counter
                return true;         // Return true indicating it's okay to proceed with empty data
            }

            try
            {
                json data = json::parse(file);
                file.close(); // Close file after parsing

                m_users.clearList(); // Use clearList()
                int maxId = 0;

                if (data.contains("users") && data["users"].is_array())
                {
                    for (const auto &userJson : data["users"])
                    {
                        User user = jsonToUser(userJson);
                        if (user.getUserId() > 0)
                        {                          // Basic validation
                            m_users.addBack(user); // Use addBack()
                            if (user.getUserId() > maxId)
                            {
                                maxId = user.getUserId();
                            }
                        }
                        else
                        {
                            std::cerr << "Warning: Invalid user ID (<=0) found while loading users. Skipped." << std::endl;
                        }
                    }
                }
                m_nextId = maxId + 1; // Set the next ID based on loaded data
                return true;
            }
            catch (const json::parse_error &e)
            {
                std::cerr << "Error: Failed to parse user JSON data: " << e.what() << " at byte " << e.byte << std::endl;
                m_users.clearList(); // Use clearList()
                m_nextId = 1;
                return false; // Indicate failure
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error loading user data: " << e.what() << std::endl;
                m_users.clearList(); // Use clearList()
                m_nextId = 1;
                return false;
            }
        }

        bool JsonUserRepository::saveAll()
        {
            json data;
            json usersJson = json::array();

            // Iterate through the linked list and serialize each user
            for (const auto &user : m_users)
            {
                usersJson.push_back(userToJson(user));
            }

            data["users"] = usersJson;
            // We might store nextId in the JSON too, but for simplicity, we recalculate on load.

            std::ofstream file(m_filePath);
            if (!file.is_open())
            {
                std::cerr << "Error: Could not open user data file for writing: " << m_filePath << std::endl;
                return false;
            }

            try
            {
                file << data.dump(2); // Pretty print with indent 2
                file.close();
                return true;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error saving user data: " << e.what() << std::endl;
                // Attempt to close the file even if writing failed
                if (file.is_open())
                {
                    file.close();
                }
                return false;
            }
        }

        // --- UserRepository Interface Implementation ---

        std::optional<User> JsonUserRepository::findById(int userId) const
        {
            for (const auto &user : m_users)
            {
                if (user.getUserId() == userId)
                {
                    return user; // Return a copy
                }
            }
            return std::nullopt;
        }

        std::optional<User> JsonUserRepository::findByUsername(const std::string &username) const
        {
            for (const auto &user : m_users)
            {
                if (user.getUsername() == username)
                {
                    return user; // Return a copy
                }
            }
            return std::nullopt;
        }

        // Changed return type to std::vector<User> to avoid deleted copy constructor issue
        std::vector<User> JsonUserRepository::findAll() const
        {
            std::vector<User> userVector;
            userVector.reserve(m_users.getSize()); // Optional optimization
            for (const auto &user : m_users)
            {
                userVector.push_back(user); // Push copies into the vector
            }
            return userVector;
        }

        bool JsonUserRepository::save(const User &userToSave)
        {
            int idToUse = userToSave.getUserId();
            bool existingFoundAndUpdated = false;

            if (idToUse > 0)
            { // If a specific ID is provided, try to find and update
                for (auto it = m_users.begin(); it != m_users.end(); ++it)
                {
                    if ((*it).getUserId() == idToUse)
                    {
                        *it = userToSave; // Update existing user
                        existingFoundAndUpdated = true;
                        break;
                    }
                }
            }

            if (!existingFoundAndUpdated)
            {                                // If not found (either ID was <=0, or ID > 0 but not in list - e.g. loading from persistence)
                User userToAdd = userToSave; // Start with a copy

                if (idToUse <= 0)
                { // It's a truly new user, assign a new ID from m_nextId
                    idToUse = m_nextId++;
                    // Create a new User object with the new ID and copy other fields
                    userToAdd = User(idToUse,
                                     userToSave.getUsername(),
                                     userToSave.getPlainTextPassword(),
                                     userToSave.getRole());
                }
                // If idToUse > 0 but not found, it means we are loading from persistence.
                // We use the idToUse (which is userToSave.getUserId())
                // and userToAdd is already a correct copy.
                m_users.addBack(userToAdd);
            }

            // Ensure m_nextId is always greater than the ID just processed or any existing max ID
            if (idToUse >= m_nextId)
            {
                m_nextId = idToUse + 1;
            }
            // Additionally, ensure m_nextId is greater than any ID in the list (e.g. after loading)
            for (const auto &u : m_users)
            {
                if (u.getUserId() >= m_nextId)
                {
                    m_nextId = u.getUserId() + 1;
                }
            }

            return saveAll();
        }

        bool JsonUserRepository::remove(int userId)
        {
            // Find the user first using the iterator
            auto it = m_users.begin();
            while (it != m_users.end())
            {
                if ((*it).getUserId() == userId)
                {
                    // Found the user, now remove it using the list's removeValue (which uses operator==)
                    // Or manually remove the node if removeValue is not suitable/available
                    // Let's assume removeValue works based on User::operator==
                    bool removed = m_users.removeValue(*it); // Try removing by value
                    if (removed)
                    {
                        return saveAll(); // Save changes if removal was successful
                    }
                    else
                    {
                        // This case should ideally not happen if we found the iterator,
                        // unless removeValue has issues or operator== is inconsistent.
                        std::cerr << "Error: Found user ID " << userId << " but could not remove from list." << std::endl;
                        return false;
                    }
                }
                ++it;
            }

            // If the loop finishes without finding the user
            return false; // User not found
        }

        int JsonUserRepository::getNextId() const
        {
            return m_nextId;
        }

    } // namespace Persistence
} // namespace RecipeApp
// Add new method implementation at the end of the file, before the closing namespace brace.

void RecipeApp::Persistence::JsonUserRepository::setNextId(int nextId)
{
    m_nextId = nextId;
}