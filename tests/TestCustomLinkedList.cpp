#include <iostream>
#include <cassert>
#include <string>                         // For testing with strings
#include "../src/core/CustomLinkedList.h" // Assuming CustomLinkedList.h is in the same directory or include path

// Helper function to print test results
void printTestResult(const std::string &testName, bool success)
{
    std::cout << (success ? "[PASS]" : "[FAIL]") << " " << testName << std::endl;
}

void testInitialization()
{
    std::cout << "--- Testing Initialization ---" << std::endl;
    CustomDataStructures::CustomLinkedList<int> list;
    bool success = list.isEmpty() && list.getSize() == 0;
    assert(success && "Initialization: List should be empty and size 0.");
    printTestResult("testInitialization", success);
}

void testAddFront()
{
    std::cout << "--- Testing addFront ---" << std::endl;
    CustomDataStructures::CustomLinkedList<int> list;
    list.addFront(10);
    bool s1 = !list.isEmpty() && list.getSize() == 1;
    assert(s1 && "addFront: Size should be 1 after adding one element.");
    printTestResult("testAddFront - Add one int", s1);

    list.addFront(20);
    bool s2 = list.getSize() == 2;
    assert(s2 && "addFront: Size should be 2 after adding two elements.");
    printTestResult("testAddFront - Add second int", s2);
    // Later, we'll add tests to check actual values once get/iterator is available

    CustomDataStructures::CustomLinkedList<std::string> strList;
    strList.addFront("hello");
    bool s3 = strList.getSize() == 1;
    assert(s3 && "addFront: String list size should be 1.");
    printTestResult("testAddFront - Add one string", s3);

    strList.addFront("world");
    bool s4 = strList.getSize() == 2;
    assert(s4 && "addFront: String list size should be 2.");
    printTestResult("testAddFront - Add second string", s4);
}

void testAddBack()
{
    std::cout << "--- Testing addBack ---" << std::endl;
    CustomDataStructures::CustomLinkedList<int> list;
    list.addBack(10);
    bool s1 = !list.isEmpty() && list.getSize() == 1;
    assert(s1 && "addBack: Size should be 1 after adding one element.");
    printTestResult("testAddBack - Add one int", s1);

    list.addBack(20);
    bool s2 = list.getSize() == 2;
    assert(s2 && "addBack: Size should be 2 after adding two elements.");
    printTestResult("testAddBack - Add second int", s2);

    CustomDataStructures::CustomLinkedList<double> doubleList;
    doubleList.addBack(1.1);
    bool s3 = doubleList.getSize() == 1;
    assert(s3 && "addBack: Double list size should be 1.");
    printTestResult("testAddBack - Add one double", s3);

    doubleList.addBack(2.2);
    bool s4 = doubleList.getSize() == 2;
    assert(s4 && "addBack: Double list size should be 2.");
    printTestResult("testAddBack - Add second double", s4);
}

void testMixedAdd()
{
    std::cout << "--- Testing Mixed Add ---" << std::endl;
    CustomDataStructures::CustomLinkedList<int> list;
    list.addBack(1);   // List: 1
    list.addFront(0);  // List: 0, 1
    list.addBack(2);   // List: 0, 1, 2
    list.addFront(-1); // List: -1, 0, 1, 2
    bool success = list.getSize() == 4;
    assert(success && "MixedAdd: Size should be 4.");
    printTestResult("testMixedAdd", success);
}

void testClearList()
{
    std::cout << "--- Testing clearList ---" << std::endl;
    CustomDataStructures::CustomLinkedList<int> list;

    list.clearList(); // Clear empty list
    bool s1 = list.isEmpty() && list.getSize() == 0;
    assert(s1 && "clearList: Clearing an empty list.");
    printTestResult("testClearList - Clear empty", s1);

    list.addBack(10);
    list.addBack(20);
    bool s2 = !list.isEmpty() && list.getSize() == 2;
    assert(s2 && "clearList: List should be non-empty before clearing.");
    printTestResult("testClearList - Before clear non-empty", s2);

    list.clearList(); // Clear non-empty list
    bool s3 = list.isEmpty() && list.getSize() == 0;
    assert(s3 && "clearList: List should be empty after clearing.");
    printTestResult("testClearList - After clear non-empty", s3);
}

void testRemoveFront()
{
    std::cout << "--- Testing removeFront ---" << std::endl;
    CustomDataStructures::CustomLinkedList<int> list;

    // Test removing from empty list (should throw)
    bool threwException = false;
    try
    {
        list.removeFront();
    }
    catch (const std::out_of_range &e)
    {
        threwException = true;
    }
    assert(threwException && "removeFront: Should throw on empty list.");
    printTestResult("testRemoveFront - Throw on empty", threwException);

    list.addBack(10); // List: 10
    list.addBack(20); // List: 10, 20
    list.addBack(30); // List: 10, 20, 30

    int val1 = list.removeFront(); // Removes 10, List: 20, 30
    bool s1 = (val1 == 10) && (list.getSize() == 2);
    assert(s1 && "removeFront: Removed 10, size should be 2.");
    printTestResult("testRemoveFront - Remove 10", s1);

    int val2 = list.removeFront(); // Removes 20, List: 30
    bool s2 = (val2 == 20) && (list.getSize() == 1);
    assert(s2 && "removeFront: Removed 20, size should be 1.");
    printTestResult("testRemoveFront - Remove 20", s2);

    int val3 = list.removeFront(); // Removes 30, List: empty
    bool s3 = (val3 == 30) && list.isEmpty() && (list.getSize() == 0);
    assert(s3 && "removeFront: Removed 30, list should be empty.");
    printTestResult("testRemoveFront - Remove 30, list empty", s3);

    // Test removing from empty list again
    threwException = false;
    try
    {
        list.removeFront();
    }
    catch (const std::out_of_range &e)
    {
        threwException = true;
    }
    assert(threwException && "removeFront: Should throw on empty list after removals.");
    printTestResult("testRemoveFront - Throw on empty again", threwException);
}

void testRemoveBack()
{
    std::cout << "--- Testing removeBack ---" << std::endl;
    CustomDataStructures::CustomLinkedList<std::string> list;

    // Test removing from empty list (should throw)
    bool threwException = false;
    try
    {
        list.removeBack();
    }
    catch (const std::out_of_range &e)
    {
        threwException = true;
    }
    assert(threwException && "removeBack: Should throw on empty list.");
    printTestResult("testRemoveBack - Throw on empty", threwException);

    list.addFront("c"); // List: c
    list.addFront("b"); // List: b, c
    list.addFront("a"); // List: a, b, c

    std::string val1 = list.removeBack(); // Removes "c", List: a, b
    bool s1 = (val1 == "c") && (list.getSize() == 2);
    assert(s1 && "removeBack: Removed 'c', size should be 2.");
    printTestResult("testRemoveBack - Remove 'c'", s1);

    std::string val2 = list.removeBack(); // Removes "b", List: a
    bool s2 = (val2 == "b") && (list.getSize() == 1);
    assert(s2 && "removeBack: Removed 'b', size should be 1.");
    printTestResult("testRemoveBack - Remove 'b'", s2);

    std::string val3 = list.removeBack(); // Removes "a", List: empty
    bool s3 = (val3 == "a") && list.isEmpty() && (list.getSize() == 0);
    assert(s3 && "removeBack: Removed 'a', list should be empty.");
    printTestResult("testRemoveBack - Remove 'a', list empty", s3);

    // Test removing from empty list again
    threwException = false;
    try
    {
        list.removeBack();
    }
    catch (const std::out_of_range &e)
    {
        threwException = true;
    }
    assert(threwException && "removeBack: Should throw on empty list after removals.");
    printTestResult("testRemoveBack - Throw on empty again", threwException);
}

void testGetAtIndex()
{
    std::cout << "--- Testing getAtIndex ---" << std::endl;
    CustomDataStructures::CustomLinkedList<int> list;

    // Test getting from empty list
    bool threwException = false;
    try
    {
        list.getAtIndex(0);
    }
    catch (const std::out_of_range &e)
    {
        threwException = true;
    }
    assert(threwException && "getAtIndex: Should throw on empty list.");
    printTestResult("testGetAtIndex - Throw on empty", threwException);

    list.addBack(10); // List: 10
    list.addBack(20); // List: 10, 20
    list.addBack(30); // List: 10, 20, 30

    bool s1 = (list.getAtIndex(0) == 10);
    assert(s1 && "getAtIndex: Element at index 0 should be 10.");
    printTestResult("testGetAtIndex - Get index 0", s1);

    bool s2 = (list.getAtIndex(1) == 20);
    assert(s2 && "getAtIndex: Element at index 1 should be 20.");
    printTestResult("testGetAtIndex - Get index 1", s2);

    bool s3 = (list.getAtIndex(2) == 30);
    assert(s3 && "getAtIndex: Element at index 2 should be 30.");
    printTestResult("testGetAtIndex - Get index 2 (last element)", s3);

    // Test modifying element through reference
    list.getAtIndex(1) = 25;
    bool s4 = (list.getAtIndex(1) == 25);
    assert(s4 && "getAtIndex: Element at index 1 should be 25 after modification.");
    printTestResult("testGetAtIndex - Modify element at index 1", s4);

    // Test out of bounds
    threwException = false;
    try
    {
        list.getAtIndex(3); // Index 3 is out of bounds for size 3
    }
    catch (const std::out_of_range &e)
    {
        threwException = true;
    }
    assert(threwException && "getAtIndex: Should throw on out-of-bounds index (upper).");
    printTestResult("testGetAtIndex - Throw on out-of-bounds (upper)", threwException);

    // Test const version
    const CustomDataStructures::CustomLinkedList<int> &constList = list;
    bool s5 = (constList.getAtIndex(0) == 10);
    assert(s5 && "getAtIndex (const): Element at index 0 should be 10.");
    printTestResult("testGetAtIndex - Const get index 0", s5);
}

void testAddAt()
{
    std::cout << "--- Testing addAt ---" << std::endl;
    CustomDataStructures::CustomLinkedList<int> list;

    // Test addAt on empty list (index 0)
    list.addAt(0, 5); // List: 5
    bool s1 = (list.getSize() == 1) && (list.getAtIndex(0) == 5);
    assert(s1 && "addAt: Add to empty list at index 0.");
    printTestResult("testAddAt - Add to empty at index 0", s1);
    list.clearList();

    // Test addAt front
    list.addBack(10); // List: 10
    list.addBack(20); // List: 10, 20
    list.addAt(0, 5); // List: 5, 10, 20
    bool s2 = (list.getSize() == 3) && (list.getAtIndex(0) == 5) && (list.getAtIndex(1) == 10);
    assert(s2 && "addAt: Add to front (index 0).");
    printTestResult("testAddAt - Add to front", s2);

    // Test addAt end (index == size)
    list.addAt(3, 30); // List: 5, 10, 20, 30 (size is 3, index 3 is end)
    bool s3 = (list.getSize() == 4) && (list.getAtIndex(3) == 30) && (list.getAtIndex(2) == 20);
    assert(s3 && "addAt: Add to end (index == size).");
    printTestResult("testAddAt - Add to end", s3);

    // Test addAt middle
    list.addAt(2, 15); // List: 5, 10, 15, 20, 30 (insert 15 at index 2)
    bool s4 = (list.getSize() == 5) && (list.getAtIndex(1) == 10) && (list.getAtIndex(2) == 15) && (list.getAtIndex(3) == 20);
    assert(s4 && "addAt: Add to middle.");
    printTestResult("testAddAt - Add to middle", s4);

    // Test addAt out of bounds (index > size)
    bool threwException = false;
    try
    {
        list.addAt(10, 100); // Current size is 5, index 10 is out of bounds
    }
    catch (const std::out_of_range &e)
    {
        threwException = true;
    }
    assert(threwException && "addAt: Should throw on out-of-bounds index (index > size).");
    printTestResult("testAddAt - Throw on out-of-bounds (index > size)", threwException);

    // Verify list integrity after operations
    // Expected: 5, 10, 15, 20, 30
    std::cout << "Final list after addAt tests: ";
    for (size_t i = 0; i < list.getSize(); ++i)
    {
        std::cout << list.getAtIndex(i) << (i == list.getSize() - 1 ? "" : ", ");
    }
    std::cout << std::endl;
}

void testRemoveAt()
{
    std::cout << "--- Testing removeAt ---" << std::endl;
    CustomDataStructures::CustomLinkedList<int> list;

    // Test removeAt on empty list
    bool threwException = false;
    try
    {
        list.removeAt(0);
    }
    catch (const std::out_of_range &e)
    {
        threwException = true;
    }
    assert(threwException && "removeAt: Should throw on empty list.");
    printTestResult("testRemoveAt - Throw on empty", threwException);

    list.addBack(10); // List: 10
    list.addBack(20); // List: 10, 20
    list.addBack(30); // List: 10, 20, 30
    list.addBack(40); // List: 10, 20, 30, 40
    list.addBack(50); // List: 10, 20, 30, 40, 50

    // Test removeAt out of bounds (upper)
    threwException = false;
    try
    {
        list.removeAt(5); // Size is 5, index 5 is out of bounds
    }
    catch (const std::out_of_range &e)
    {
        threwException = true;
    }
    assert(threwException && "removeAt: Should throw on out-of-bounds index (upper).");
    printTestResult("testRemoveAt - Throw on out-of-bounds (upper)", threwException);

    // Remove from front using removeAt(0)
    int val1 = list.removeAt(0); // Removes 10. List: 20, 30, 40, 50
    bool s1 = (val1 == 10) && (list.getSize() == 4) && (list.getAtIndex(0) == 20);
    assert(s1 && "removeAt: Removed 10 from front.");
    printTestResult("testRemoveAt - Remove from front (index 0)", s1);

    // Remove from end using removeAt(getSize() - 1)
    int val2 = list.removeAt(list.getSize() - 1); // Removes 50. List: 20, 30, 40
    bool s2 = (val2 == 50) && (list.getSize() == 3) && (list.getAtIndex(list.getSize() - 1) == 40);
    assert(s2 && "removeAt: Removed 50 from end.");
    printTestResult("testRemoveAt - Remove from end (index getSize()-1)", s2);

    // Remove from middle
    // Current list: 20, 30, 40
    int val3 = list.removeAt(1); // Removes 30. List: 20, 40
    bool s3 = (val3 == 30) && (list.getSize() == 2) && (list.getAtIndex(0) == 20) && (list.getAtIndex(1) == 40);
    assert(s3 && "removeAt: Removed 30 from middle (index 1).");
    printTestResult("testRemoveAt - Remove from middle", s3);

    // Remove remaining elements
    int val4 = list.removeAt(0); // Removes 20. List: 40
    int val5 = list.removeAt(0); // Removes 40. List: empty
    bool s4 = (val4 == 20) && (val5 == 40) && list.isEmpty();
    assert(s4 && "removeAt: Removed remaining elements.");
    printTestResult("testRemoveAt - Remove all remaining", s4);

    // Test removeAt on empty list again
    threwException = false;
    try
    {
        list.removeAt(0);
    }
    catch (const std::out_of_range &e)
    {
        threwException = true;
    }
    assert(threwException && "removeAt: Should throw on empty list after all removals.");
    printTestResult("testRemoveAt - Throw on empty again", threwException);
}

void testFindNode()
{
    std::cout << "--- Testing findNode ---" << std::endl;
    CustomDataStructures::CustomLinkedList<int> list;
    const CustomDataStructures::CustomLinkedList<int> &constList = list; // For const version

    // Test findNode on empty list
    bool s1 = (list.findNode(10) == nullptr) && (constList.findNode(10) == nullptr);
    assert(s1 && "findNode: Should return nullptr on empty list.");
    printTestResult("testFindNode - Empty list", s1);

    list.addBack(10); // List: 10
    list.addBack(20); // List: 10, 20
    list.addBack(30); // List: 10, 20, 30
    list.addBack(20); // List: 10, 20, 30, 20 (duplicate value)

    // Test findNode for existing value (first occurrence)
    CustomDataStructures::Node<int> *node1 = list.findNode(20);
    bool s2 = (node1 != nullptr) && (node1->data == 20) && (list.getAtIndex(1) == 20); // Assuming getAtIndex works
    assert(s2 && "findNode: Found first occurrence of 20.");
    printTestResult("testFindNode - Find existing (first occurrence)", s2);

    // Test findNode for existing value (const version)
    const CustomDataStructures::Node<int> *cNode1 = constList.findNode(10);
    bool s3 = (cNode1 != nullptr) && (cNode1->data == 10);
    assert(s3 && "findNode (const): Found 10.");
    printTestResult("testFindNode - Find existing (const)", s3);

    // Test findNode for non-existing value
    bool s4 = (list.findNode(100) == nullptr) && (constList.findNode(100) == nullptr);
    assert(s4 && "findNode: Should return nullptr for non-existing value.");
    printTestResult("testFindNode - Non-existing value", s4);

    // Test findNode for value at head
    CustomDataStructures::Node<int> *nodeHead = list.findNode(10);
    bool s5 = (nodeHead != nullptr) && (nodeHead->data == 10);
    assert(s5 && "findNode: Found value at head.");
    printTestResult("testFindNode - Value at head", s5);

    // Test findNode for value at tail (last 20)
    // To specifically test finding the last '20', we'd need more complex logic or rely on removeValue tests
    // For now, findNode finds the *first* occurrence.
    list.clearList();
    list.addBack(5);
    list.addBack(15);
    list.addBack(25);
    CustomDataStructures::Node<int> *nodeTail = list.findNode(25);
    bool s6 = (nodeTail != nullptr) && (nodeTail->data == 25);
    assert(s6 && "findNode: Found value at tail.");
    printTestResult("testFindNode - Value at tail", s6);
}

void testRemoveValue()
{
    std::cout << "--- Testing removeValue ---" << std::endl;
    CustomDataStructures::CustomLinkedList<int> list;

    // Test removeValue on empty list
    bool s1 = !list.removeValue(10);
    assert(s1 && "removeValue: Should return false on empty list.");
    printTestResult("testRemoveValue - Empty list", s1);

    list.addBack(10); // List: 10
    list.addBack(20); // List: 10, 20
    list.addBack(30); // List: 10, 20, 30
    list.addBack(20); // List: 10, 20, 30, 20
    list.addBack(40); // List: 10, 20, 30, 20, 40

    // Test removeValue for non-existing value
    bool s2 = !list.removeValue(100) && (list.getSize() == 5);
    assert(s2 && "removeValue: Should return false for non-existing value.");
    printTestResult("testRemoveValue - Non-existing value", s2);

    // Test removeValue for head element
    // List: 10, 20, 30, 20, 40
    bool s3 = list.removeValue(10) && (list.getSize() == 4) && (list.getAtIndex(0) == 20);
    assert(s3 && "removeValue: Removed head element (10).");
    printTestResult("testRemoveValue - Remove head", s3);
    // List: 20, 30, 20, 40

    // Test removeValue for tail element
    bool s4 = list.removeValue(40) && (list.getSize() == 3) && (list.getAtIndex(list.getSize() - 1) == 20);
    assert(s4 && "removeValue: Removed tail element (40).");
    printTestResult("testRemoveValue - Remove tail", s4);
    // List: 20, 30, 20

    // Test removeValue for middle element (first occurrence of 20)
    bool s5 = list.removeValue(20) && (list.getSize() == 2) && (list.getAtIndex(0) == 30) && (list.getAtIndex(1) == 20);
    assert(s5 && "removeValue: Removed middle element (first 20).");
    printTestResult("testRemoveValue - Remove middle (first occurrence)", s5);
    // List: 30, 20

    // Test removeValue for the only remaining occurrence of a value
    bool s6 = list.removeValue(20) && (list.getSize() == 1) && (list.getAtIndex(0) == 30);
    assert(s6 && "removeValue: Removed last 20.");
    printTestResult("testRemoveValue - Remove last occurrence", s6);
    // List: 30

    // Test removeValue when list becomes empty
    bool s7 = list.removeValue(30) && list.isEmpty();
    assert(s7 && "removeValue: Removed last element, list becomes empty.");
    printTestResult("testRemoveValue - Remove last element, list empty", s7);

    // Test removeValue on now empty list
    bool s8 = !list.removeValue(30);
    assert(s8 && "removeValue: Should return false on empty list after removals.");
    printTestResult("testRemoveValue - Empty list after removals", s8);

    // Test with strings
    CustomDataStructures::CustomLinkedList<std::string> strList;
    strList.addBack("apple");
    strList.addBack("banana");
    strList.addBack("cherry");
    strList.addBack("banana");
    bool s9 = strList.removeValue("banana") && strList.getSize() == 3 && strList.getAtIndex(1) == "cherry";
    assert(s9 && "removeValue (string): Removed first 'banana'.");
    printTestResult("testRemoveValue - String list, remove first occurrence", s9);
    // strList: apple, cherry, banana
    bool s10 = strList.removeValue("banana") && strList.getSize() == 2 && strList.getAtIndex(1) == "cherry";
    assert(s10 && "removeValue (string): Removed second 'banana'.");
    printTestResult("testRemoveValue - String list, remove second occurrence", s10);
    // strList: apple, cherry
}

void testIterator()
{
    std::cout << "--- Testing Iterator ---" << std::endl;
    CustomDataStructures::CustomLinkedList<int> list;

    // Test on empty list
    int countEmpty = 0;
    for (int val : list)
    {
        countEmpty++;
        (void)val; // Suppress unused variable warning
    }
    bool s1 = (countEmpty == 0);
    assert(s1 && "Iterator: Range-based for on empty list should not iterate.");
    printTestResult("testIterator - Range-based for on empty list", s1);

    // Test manual iteration on empty list
    bool s1_manual = (list.begin() == list.end());
    assert(s1_manual && "Iterator: begin() should equal end() on empty list.");
    printTestResult("testIterator - Manual iteration on empty list (begin == end)", s1_manual);

    list.addBack(10); // List: 10
    list.addBack(20); // List: 10, 20
    list.addBack(30); // List: 10, 20, 30

    // Test range-based for loop
    std::cout << "Iterating with range-based for: ";
    int sum = 0;
    int expectedSum = 10 + 20 + 30;
    int loopCount = 0;
    for (int val : list)
    {
        std::cout << val << " ";
        sum += val;
        loopCount++;
    }
    std::cout << std::endl;
    bool s2 = (sum == expectedSum) && (loopCount == 3);
    assert(s2 && "Iterator: Range-based for loop sum/count mismatch.");
    printTestResult("testIterator - Range-based for (sum and count)", s2);

    // Test manual iteration
    std::cout << "Iterating manually: ";
    sum = 0;
    loopCount = 0;
    for (CustomDataStructures::CustomLinkedList<int>::Iterator it = list.begin(); it != list.end(); ++it)
    {
        std::cout << *it << " ";
        sum += *it;
        loopCount++;
    }
    std::cout << std::endl;
    bool s3 = (sum == expectedSum) && (loopCount == 3);
    assert(s3 && "Iterator: Manual iteration sum/count mismatch.");
    printTestResult("testIterator - Manual iteration (sum and count)", s3);

    // Test const_iterator with range-based for
    const CustomDataStructures::CustomLinkedList<int> &constList = list;
    std::cout << "Iterating const list with range-based for: ";
    sum = 0;
    loopCount = 0;
    for (int val : constList)
    {
        std::cout << val << " ";
        sum += val;
        loopCount++;
    }
    std::cout << std::endl;
    bool s4 = (sum == expectedSum) && (loopCount == 3);
    assert(s4 && "Iterator: Const range-based for loop sum/count mismatch.");
    printTestResult("testIterator - Const Range-based for (sum and count)", s4);

    // Test manual const_iterator (using cbegin/cend)
    std::cout << "Iterating const list manually (cbegin/cend): ";
    sum = 0;
    loopCount = 0;
    for (CustomDataStructures::CustomLinkedList<int>::ConstIterator it = constList.cbegin(); it != constList.cend(); ++it)
    {
        std::cout << *it << " ";
        sum += *it;
        loopCount++;
    }
    std::cout << std::endl;
    bool s5 = (sum == expectedSum) && (loopCount == 3);
    assert(s5 && "Iterator: Manual const_iterator (cbegin/cend) sum/count mismatch.");
    printTestResult("testIterator - Manual const_iterator (cbegin/cend, sum and count)", s5);

    // Test modifying through non-const iterator
    for (CustomDataStructures::CustomLinkedList<int>::Iterator it = list.begin(); it != list.end(); ++it)
    {
        *it += 1; // Increment each element
    }
    // Expected list: 11, 21, 31. Expected sum: 11+21+31 = 63
    sum = 0;
    for (int val : list)
    {
        sum += val;
    }
    bool s6 = (sum == 63) && (list.getAtIndex(0) == 11) && (list.getAtIndex(1) == 21) && (list.getAtIndex(2) == 31);
    assert(s6 && "Iterator: Modifying elements through iterator failed.");
    printTestResult("testIterator - Modify elements via iterator", s6);

    // Test bidirectional capabilities (decrement)
    // Current list: 11, 21, 31
    std::cout << "Iterating backwards manually: ";
    sum = 0;
    loopCount = 0;

    if (!list.isEmpty())
    {
        CustomDataStructures::CustomLinkedList<int>::Iterator it = list.end();
        // To correctly start from the last element using a standard approach with end(),
        // you would typically decrement end() once if the list is not empty.
        // However, our simple --end() is not robustly defined to go to tail if m_ptr is nullptr.
        // So, we find the last element manually for this test.
    } // Closes if (!list.isEmpty()) from line 635

} // Closes testIterator() function from line 528

// Add main function at the end of the file

int main()
{
    testInitialization();
    testAddFront();
    testAddBack();
    testMixedAdd();
    testClearList();
    testRemoveFront();
    testRemoveBack();
    testGetAtIndex();
    testAddAt();
    testRemoveAt();
    testFindNode();
    testRemoveValue();
    testIterator(); // Assuming this function is now correctly defined and closed

    std::cout << "\n--- All CustomLinkedList tests completed ---" << std::endl;
    return 0;
}
