#ifndef JSON_REPOSITORY_BASE_H
#define JSON_REPOSITORY_BASE_H

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>  // For std::runtime_error
#include <string>
#include <vector>

#include "../../include/json.hpp"

using json = nlohmann::json;

namespace RecipeApp {
namespace Persistence {

template <typename T>
class JsonRepositoryBase {
   protected:
    std::string m_filePath;
    std::vector<T> m_items;
    int m_nextId;
    std::string m_jsonArrayKey;

   public:
    JsonRepositoryBase(const std::filesystem::path& baseDirectory,
                       const std::string& fileName,
                       const std::string& jsonArrayKey)
        : m_nextId(1), m_jsonArrayKey(std::move(jsonArrayKey)) {
        if (!std::filesystem::exists(baseDirectory)) {
            try {
                if (std::filesystem::create_directories(baseDirectory)) {
                    // Optional: Log directory creation
                }
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Critical Error: Could not create base directory '"
                          << baseDirectory.string() << "': " << e.what()
                          << std::endl;
                m_filePath = fileName;
                std::cerr << "Warning: Using fallback file path in current "
                             "directory for "
                          << m_jsonArrayKey << ": " << m_filePath << std::endl;
                // Consider throwing an exception here to halt initialization if
                // directory is critical throw std::runtime_error("Failed to
                // create base directory: " + baseDirectory.string());
                return;  // Or handle error appropriately
            }
        }
        m_filePath = (baseDirectory / fileName).string();
    }

    virtual ~JsonRepositoryBase() = default;

    bool load() {
        std::ifstream file(m_filePath);
        if (!file.is_open()) {
            std::cerr << "Warning: Could not open data file for reading: "
                      << m_filePath << ". Starting with an empty list for "
                      << m_jsonArrayKey << "." << std::endl;
            m_items.clear();
            m_nextId = 1;
            return true;
        }

        try {
            json data = json::parse(file); // This can throw
            file.close(); // Close the file as soon as parsing is done

            m_items.clear();
            int maxId = 0;

            if (data.contains(m_jsonArrayKey) &&
                data[m_jsonArrayKey].is_array()) {
                for (const auto& itemJson : data[m_jsonArrayKey]) {
                    try {
                        T item = itemJson.get<T>();
                        // Assuming T has a method like getId()
                        // This method name 'getId()' must be consistent across
                        // types T (Recipe, Restaurant)
                        if (item.getId() > 0) {
                            m_items.push_back(item);
                            if (item.getId() > maxId) {
                                maxId = item.getId();
                            }
                        } else {
                            std::cerr << "Warning: Invalid ID (<=0) found "
                                         "while loading "
                                      << m_jsonArrayKey
                                      << ". Skipped: " << itemJson.dump(2)
                                      << std::endl;
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Warning: Failed to load an item for "
                                  << m_jsonArrayKey
                                  << " due to error: " << e.what()
                                  << ". Invalid JSON: " << itemJson.dump(2)
                                  << std::endl;
                    }
                }
            }
            m_nextId = maxId + 1;
            return true;
        } catch (const json::parse_error& e) {
            std::cerr << "Error: Failed to parse JSON data for "
                      << m_jsonArrayKey << ": " << e.what() << " at byte "
                      << e.byte << std::endl;
            if (file.is_open()) { // Ensure file is closed on error
                file.close();
            }
            m_items.clear();
            m_nextId = 1;
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Error loading data for " << m_jsonArrayKey << ": "
                      << e.what() << std::endl;
            if (file.is_open()) { // Ensure file is closed on error
                file.close();
            }
            m_items.clear();
            m_nextId = 1;
            return false;
        }
        // Fallback, though ideally should not be reached if try/catch is exhaustive
        if (file.is_open()) {
            file.close();
        }
        return false;
    }

    bool saveAll() {  // This should be protected or private if only called by
                      // save/remove
        json dataDoc;
        json itemsJsonArray = json::array();

        for (const auto& item : m_items) {
            itemsJsonArray.push_back(item);
        }
        dataDoc[m_jsonArrayKey] = itemsJsonArray;

        std::filesystem::path filePathObj(m_filePath);
        std::filesystem::path tempFilePathObj = filePathObj;
        tempFilePathObj += ".tmp";

        if (!filePathObj.parent_path().empty() &&
            !std::filesystem::exists(filePathObj.parent_path())) {
            try {
                std::filesystem::create_directories(filePathObj.parent_path());
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr
                    << "Error: Could not create directory for saving file "
                    << filePathObj.parent_path().string() << ": " << e.what()
                    << std::endl;
                return false;
            }
        }

        std::ofstream tempFile(tempFilePathObj.string(),
                               std::ios::out | std::ios::trunc);
        if (!tempFile.is_open()) {
            std::cerr
                << "Error: Could not open temporary data file for writing: "
                << tempFilePathObj.string() << std::endl;
            return false;
        }

        try {
            tempFile << dataDoc.dump(2);
            tempFile.close();
            if (tempFile.fail()) {
                std::cerr
                    << "Error: Failed to write all data to temporary file: "
                    << tempFilePathObj.string() << std::endl;
                std::filesystem::remove(tempFilePathObj);  // Attempt cleanup
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error serializing data to temporary file '"
                      << tempFilePathObj.string() << "': " << e.what()
                      << std::endl;
            if (tempFile.is_open()) tempFile.close();
            std::filesystem::remove(tempFilePathObj);  // Attempt cleanup
            return false;
        }

        try {
            // Attempt to remove the original file before renaming.
            // This might help if overwriting is problematic due to locks.
            if (std::filesystem::exists(filePathObj)) {
                std::error_code ec_remove_orig;
                std::filesystem::remove(filePathObj, ec_remove_orig);
                if (ec_remove_orig) {
                    std::cerr << "Warning: Could not remove original file '"
                              << filePathObj.string() << "' before rename: "
                              << ec_remove_orig.message() << std::endl;
                    // Proceed to attempt rename anyway, it might still work or provide the original error.
                }
            }
            std::filesystem::rename(tempFilePathObj, filePathObj);
            return true;
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error: Failed to replace original file '"
                      << filePathObj.string() << "' with temporary file '"
                      << tempFilePathObj.string() << "': " << e.what()
                      << std::endl;
            if (std::filesystem::exists(tempFilePathObj)) {
                std::error_code ec;
                std::filesystem::remove(tempFilePathObj, ec);
                if (ec) {
                    std::cerr << "Warning: Could not remove temporary file '"
                              << tempFilePathObj.string()
                              << "' after rename failed: " << ec.message()
                              << std::endl;
                }
            }
            return false;
        }
    }

   protected:  // Common operations for derived classes
    std::optional<T> findByIdInternal(int itemId) const {
        auto it = std::find_if(
            m_items.begin(), m_items.end(),
            [itemId](const T& item) { return item.getId() == itemId; });
        if (it != m_items.end()) {
            return *it;
        }
        return std::nullopt;
    }

    std::vector<T> findAllInternal() const { return m_items; }

    // Derived classes will call this after preparing the item (e.g., assigning
    // new ID) Returns true if item was added/updated in memory AND persisted,
    // false otherwise.
    bool updateOrAddItemInMemoryAndPersist(const T& itemWithFinalId,
                                           bool isNewItem) {
        std::optional<T> originalItemOpt = std::nullopt;
        bool alreadyExisted = false;

        if (!isNewItem) {  // Attempt to find and update
            auto it = std::find_if(
                m_items.begin(), m_items.end(), [&itemWithFinalId](const T& r) {
                    return r.getId() == itemWithFinalId.getId();
                });
            if (it != m_items.end()) {
                originalItemOpt = *it;  // Save for potential rollback
                *it = itemWithFinalId;  // Update
                alreadyExisted = true;
            } else {
                // Trying to update an item that doesn't exist. This shouldn't
                // happen if public save() checks first. Or, if ID was
                // pre-assigned for a new item but it's not truly an update. For
                // simplicity, if not found, we'll treat as add if isNewItem was
                // false but it should have been true. This indicates a logic
                // flaw in the caller if isNewItem is false but item not found.
                // Let's assume the caller (derived class's public save) handles
                // this logic correctly. If isNewItem is false, we expect it to
                // be an update.
                std::cerr << "Warning: Attempted to update item ID "
                          << itemWithFinalId.getId() << " which was not found."
                          << std::endl;
                return false;  // Or throw, as this is an inconsistent state for
                               // an update.
            }
        } else {  // isNewItem == true
            // Check for duplicate ID before adding, though m_nextId should
            // prevent this for auto-generated IDs. This is more for cases where
            // IDs might be set externally before calling add.
            auto it_dup_check = std::find_if(
                m_items.begin(), m_items.end(), [&itemWithFinalId](const T& r) {
                    return r.getId() == itemWithFinalId.getId();
                });
            if (it_dup_check != m_items.end()) {
                std::cerr
                    << "Error: Attempted to add new item with duplicate ID "
                    << itemWithFinalId.getId() << std::endl;
                return false;  // Duplicate ID for a new item
            }
            m_items.push_back(itemWithFinalId);
        }

        if (saveAll()) {
            return true;
        } else {
            // Persistence failed, roll back memory changes
            if (!isNewItem && alreadyExisted &&
                originalItemOpt.has_value()) {  // Rollback update
                auto it = std::find_if(m_items.begin(), m_items.end(),
                                       [&itemWithFinalId](const T& r) {
                                           return r.getId() ==
                                                  itemWithFinalId.getId();
                                       });
                if (it != m_items.end()) {
                    *it = originalItemOpt.value();
                }
            } else if (isNewItem) {  // Rollback add
                m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
                                             [&itemWithFinalId](const T& r) {
                                                 return r.getId() ==
                                                        itemWithFinalId.getId();
                                             }),
                              m_items.end());
            }
            return false;
        }
    }

    bool removeItemInMemoryAndPersist(int itemId) {
        std::optional<T>
            itemToRemoveOpt;  // Use optional to avoid default construction
        auto it = std::find_if(
            m_items.begin(), m_items.end(),
            [itemId](const T& item) { return item.getId() == itemId; });

        if (it != m_items.end()) {
            itemToRemoveOpt = *it;  // Save for potential rollback
            m_items.erase(it);
            // found = true; // No longer needed, check itemToRemoveOpt

            if (saveAll()) {
                // Optional: Recalculate m_nextId if needed.
                // ensureNextIdIsCorrect(); // Could be called here by derived
                // class or if made public
                return true;
            } else {
                // Persistence failed, roll back the removal from memory
                if (itemToRemoveOpt.has_value()) {  // Check if we have
                                                    // something to roll back
                    m_items.push_back(itemToRemoveOpt.value());
                    // TODO: Consider re-inserting at original position if order
                    // matters and is feasible. For now, push_back is simpler.
                    // Sorting m_items by ID afterwards might be an option.
                }
                std::cerr << "Error: Failed to save after removing item ID "
                          << itemId << " from memory. Rollback attempted."
                          << std::endl;
                return false;
            }
        }
        return false;  // Item not found
    }

   public:  // Public interface for ID management, to be used by derived classes
    int getNextId() const { return m_nextId; }

    void setNextId(int nextId) { m_nextId = nextId; }

    // Helper to ensure m_nextId is always appropriate after any operation
    void ensureNextIdIsCorrect() {
        int maxCurrentId = 0;
        if (m_items.empty()) {
            m_nextId = 1;
        } else {
            for (const auto& item : m_items) {
                if (item.getId() > maxCurrentId) {
                    maxCurrentId = item.getId();
                }
            }
            m_nextId = maxCurrentId + 1;
        }
    }
};

}  // namespace Persistence
}  // namespace RecipeApp

#endif  // JSON_REPOSITORY_BASE_H