#ifndef CUSTOM_LINKED_LIST_H
#define CUSTOM_LINKED_LIST_H

#include <cstddef>   // For size_t
#include <stdexcept> // For std::out_of_range
#include <iostream>  // For displayAll
#include <utility>   // For std::swap (used in move assignment)

/**
 * @brief Namespace for custom data structures.
 */
namespace CustomDataStructures
{

    /**
     * @brief A template class for a node in a doubly linked list.
     * @tparam T The type of data stored in the node.
     */
    template <typename T>
    struct Node
    {
        T data;     ///< Data stored in the node.
        Node *prev; ///< Pointer to the previous node.
        Node *next; ///< Pointer to the next node.

        /**
         * @brief Constructs a new Node.
         * @param val The data to store in the node.
         * @param p Pointer to the previous node (default: nullptr).
         * @param n Pointer to the next node (default: nullptr).
         */
        Node(const T &val, Node *p = nullptr, Node *n = nullptr)
            : data(val), prev(p), next(n) {}
    };

    /**
     * @brief A template class for a custom-implemented doubly linked list.
     * @tparam T The type of data stored in the list.
     */
    template <typename T>
    class CustomLinkedList
    {
    public:
        /**
         * @brief Default constructor. Initializes an empty list.
         */
        CustomLinkedList() : head(nullptr), tail(nullptr), listSize(0) {}

        /**
         * @brief Destructor. Frees all memory allocated by the list.
         */
        ~CustomLinkedList()
        {
            clearList();
        }

        // Copy constructor
        CustomLinkedList(const CustomLinkedList &other) : head(nullptr), tail(nullptr), listSize(0)
        {
            Node<T> *current = other.head;
            while (current != nullptr)
            {
                this->addBack(current->data); // addBack creates new nodes
                current = current->next;
            }
        }

        // Move constructor
        CustomLinkedList(CustomLinkedList &&other) noexcept
            : head(other.head), tail(other.tail), listSize(other.listSize)
        {
            // Leave other in a valid but empty state
            other.head = nullptr;
            other.tail = nullptr;
            other.listSize = 0;
        }

        // Copy assignment operator (basic implementation, can be improved with copy-and-swap)
        CustomLinkedList &operator=(const CustomLinkedList &other)
        {
            if (this != &other) // Protect against self-assignment
            {
                clearList(); // Clear current contents
                Node<T> *current = other.head;
                while (current != nullptr)
                {
                    this->addBack(current->data);
                    current = current->next;
                }
            }
            return *this;
        }

        // Move assignment operator
        CustomLinkedList &operator=(CustomLinkedList &&other) noexcept
        {
            if (this != &other) // Protect against self-assignment
            {
                clearList(); // Clear current contents before taking ownership

                // Transfer ownership
                head = other.head;
                tail = other.tail;
                listSize = other.listSize;

                // Leave other in a valid but empty state
                other.head = nullptr;
                other.tail = nullptr;
                other.listSize = 0;
            }
            return *this;
        }


        /**
         * @brief Adds an element to the front of the list.
         * @param data The data to add.
         */
        void addFront(const T &data)
        {
            Node<T> *newNode = new Node<T>(data, nullptr, head);
            if (isEmpty())
            {
                head = tail = newNode;
            }
            else
            {
                head->prev = newNode;
                head = newNode;
            }
            listSize++;
        }

        /**
         * @brief Adds an element to the back of the list.
         * @param data The data to add.
         */
        void addBack(const T &data)
        {
            Node<T> *newNode = new Node<T>(data, tail, nullptr);
            if (isEmpty())
            {
                head = tail = newNode;
            }
            else
            {
                tail->next = newNode;
                tail = newNode;
            }
            listSize++;
        }

        /**
         * @brief Checks if the list is empty.
         * @return True if the list is empty, false otherwise.
         */
        bool isEmpty() const
        {
            return listSize == 0;
        }

        /**
         * @brief Gets the current number of elements in the list.
         * @return The size of the list.
         */
        size_t getSize() const
        {
            return listSize;
        }

        /**
         * @brief Removes all elements from the list and frees memory.
         */
        void clearList()
        {
            Node<T> *current = head;
            while (current != nullptr)
            {
                Node<T> *nextNode = current->next;
                delete current;
                current = nextNode;
            }
            head = nullptr;
            tail = nullptr;
            listSize = 0;
        }

        T removeFront()
        {
            if (isEmpty())
            {
                throw std::out_of_range("Cannot removeFront from an empty list.");
            }
            Node<T> *oldHead = head;
            T data = oldHead->data;
            head = head->next;
            if (head != nullptr)
            {
                head->prev = nullptr;
            }
            else
            {
                tail = nullptr;
            }
            delete oldHead;
            listSize--;
            return data;
        }

        T removeBack()
        {
            if (isEmpty())
            {
                throw std::out_of_range("Cannot removeBack from an empty list.");
            }
            Node<T> *oldTail = tail;
            T data = oldTail->data;
            tail = tail->prev;
            if (tail != nullptr)
            {
                tail->next = nullptr;
            }
            else
            {
                head = nullptr;
            }
            delete oldTail;
            listSize--;
            return data;
        }

        T &getAtIndex(size_t index)
        {
            if (index >= listSize)
            {
                throw std::out_of_range("Index out of range in getAtIndex.");
            }
            Node<T> *current;
            if (index < listSize / 2)
            {
                current = head;
                for (size_t i = 0; i < index; ++i)
                {
                    current = current->next;
                }
            }
            else
            {
                current = tail;
                for (size_t i = 0; i < listSize - 1 - index; ++i)
                {
                    current = current->prev;
                }
            }
            return current->data;
        }

        const T &getAtIndex(size_t index) const
        {
            if (index >= listSize)
            {
                throw std::out_of_range("Index out of range in getAtIndex (const).");
            }
            const Node<T> *current; 
            if (index < listSize / 2)
            {
                current = head;
                for (size_t i = 0; i < index; ++i)
                {
                    current = current->next;
                }
            }
            else
            {
                current = tail;
                for (size_t i = 0; i < listSize - 1 - index; ++i)
                {
                    current = current->prev;
                }
            }
            return current->data;
        }

        void addAt(size_t index, const T &data)
        {
            if (index > listSize)
            { 
                throw std::out_of_range("Index out of range in addAt.");
            }
            if (index == 0)
            {
                addFront(data);
                return;
            }
            if (index == listSize)
            {
                addBack(data);
                return;
            }

            Node<T> *current = head;
            for (size_t i = 0; i < index - 1; ++i)
            { 
                current = current->next;
            }
            Node<T> *newNode = new Node<T>(data, current, current->next);
            current->next->prev = newNode;
            current->next = newNode;
            listSize++;
        }

        T removeAt(size_t index)
        {
            if (index >= listSize)
            {
                throw std::out_of_range("Index out of range in removeAt.");
            }
            if (index == 0)
            {
                return removeFront();
            }
            if (index == listSize - 1)
            {
                return removeBack();
            }

            Node<T> *current = head;
            for (size_t i = 0; i < index; ++i)
            {
                current = current->next;
            }
            T data = current->data;
            current->prev->next = current->next;
            current->next->prev = current->prev;
            delete current;
            listSize--;
            return data;
        }

        Node<T> *findNode(const T &value) const
        {
            Node<T> *current = head;
            while (current != nullptr)
            {
                if (current->data == value)
                {
                    return current;
                }
                current = current->next;
            }
            return nullptr;
        }

        bool removeValue(const T &value)
        {
            Node<T> *nodeToRemove = findNode(value);
            if (nodeToRemove == nullptr)
            {
                return false; 
            }

            if (nodeToRemove == head)
            {
                removeFront();
            }
            else if (nodeToRemove == tail)
            {
                removeBack();
            }
            else
            {
                nodeToRemove->prev->next = nodeToRemove->next;
                nodeToRemove->next->prev = nodeToRemove->prev;
                delete nodeToRemove;
                listSize--;
            }
            return true;
        }

        class Iterator
        {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T *;
            using reference = T &;

            Iterator(Node<T> *ptr = nullptr) : m_ptr(ptr) {}

            reference operator*() const { return m_ptr->data; }
            pointer operator->() const { return &(m_ptr->data); }

            Iterator &operator++()
            {
                if (m_ptr)
                    m_ptr = m_ptr->next;
                return *this;
            }

            Iterator operator++(int)
            {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            Iterator &operator--()
            {
                if (m_ptr)
                { 
                    m_ptr = m_ptr->prev;
                }
                return *this;
            }

            Iterator operator--(int)
            {
                Iterator tmp = *this;
                --(*this);
                return tmp;
            }

            friend bool operator==(const Iterator &a, const Iterator &b) { return a.m_ptr == b.m_ptr; }
            friend bool operator!=(const Iterator &a, const Iterator &b) { return a.m_ptr != b.m_ptr; }

        private:
            Node<T> *m_ptr;
        };

        class ConstIterator
        {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T *;   
            using reference = const T &; 

            ConstIterator(const Node<T> *ptr = nullptr) : m_ptr(ptr) {}

            reference operator*() const { return m_ptr->data; }
            pointer operator->() const { return &(m_ptr->data); }

            ConstIterator &operator++()
            {
                if (m_ptr)
                    m_ptr = m_ptr->next;
                return *this;
            }

            ConstIterator operator++(int)
            {
                ConstIterator tmp = *this;
                ++(*this);
                return tmp;
            }

            ConstIterator &operator--()
            {
                if (m_ptr)
                {
                    m_ptr = m_ptr->prev;
                }
                return *this;
            }

            ConstIterator operator--(int)
            {
                ConstIterator tmp = *this;
                --(*this);
                return tmp;
            }

            friend bool operator==(const ConstIterator &a, const ConstIterator &b) { return a.m_ptr == b.m_ptr; }
            friend bool operator!=(const ConstIterator &a, const ConstIterator &b) { return a.m_ptr != b.m_ptr; }

        private:
            const Node<T> *m_ptr; 
        };

        Iterator begin() { return Iterator(head); }
        Iterator end() { return Iterator(nullptr); }
        ConstIterator begin() const { return ConstIterator(head); }
        ConstIterator end() const { return ConstIterator(nullptr); }
        ConstIterator cbegin() const { return ConstIterator(head); }
        ConstIterator cend() const { return ConstIterator(nullptr); }

    private:
        Node<T> *head;   
        Node<T> *tail;   
        size_t listSize; 
    };

} // namespace CustomDataStructures

#endif // CUSTOM_LINKED_LIST_H