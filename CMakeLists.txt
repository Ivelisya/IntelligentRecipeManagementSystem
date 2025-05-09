# CMake 最低版本要求 (可以根据您的 CMake 版本调整)
cmake_minimum_required(VERSION 3.10) 

# 项目名称和语言
project(RecipeCliApp LANGUAGES CXX)

# 包含 FetchContent 模块
include(FetchContent)

# 设置 C++ 标准 (例如 C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- MSVC 运行时库设置 ---
if(MSVC)
  # 为所有目标设置默认的 MSVC 运行时库
  # 这有助于避免在链接不同库（如 GTest）时出现运行时库不匹配的问题
  # "MultiThreadedDebugDLL" 对应 /MDd (用于 Debug 构建)
  # "MultiThreadedDLL" 对应 /MD (用于 Release 构建)
  # 其他选项包括 "MultiThreadedDebug" (/MTd) 和 "MultiThreaded" (/MT) (用于静态链接运行时库)
  # 通常推荐使用 DLL 版本以减小可执行文件大小并共享运行时库
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  message(STATUS "MSVC Runtime Library set to: ${CMAKE_MSVC_RUNTIME_LIBRARY}")

  # (可选) 如果你想强制所有通过 FetchContent 或 add_subdirectory 添加的子项目也遵循这个设置，
  # 你可以设置一个策略。但这通常不是必需的，因为子项目应该尊重父项目的设置。
  # cmake_policy(SET CMP0091 NEW) # MSVC runtime library flags are selected by an abstraction.
endif()

# --- 添加包含目录 ---
# 添加 'include' 目录 (包含 json.hpp)
# 添加当前源文件目录 (包含 Recipe.h, RecipeManager.h 等)
# Global include_directories (lines 15-19) removed.
# Modern CMake prefers target_include_directories for better control over include paths.
# The include paths are now exclusively managed by target_include_directories for the recipe_logic_dll target.

# --- 定义可执行文件的源文件 ---
set(EXECUTABLE_SOURCES
    src/main.cpp                     # CLI 入口文件
    src/logic/recipe/RecipeManager.cpp
    src/domain/recipe/Recipe.cpp
    src/logic/restaurant/RestaurantManager.cpp
    src/domain/restaurant/Restaurant.cpp
    src/logic/user/UserManager.cpp
    src/domain/user/User.cpp
    src/persistence/PersistenceManager.cpp # RESTORED
    src/persistence/JsonUserRepository.cpp   # Added JsonUserRepository
    src/persistence/JsonRecipeRepository.cpp # Added JsonRecipeRepository
    src/persistence/JsonRestaurantRepository.cpp # Added JsonRestaurantRepository
    src/cli/CliUtils.cpp             # CLI 辅助函数
    src/cli/handlers/UserCommandHandler.cpp # 用户命令处理程序
    src/cli/handlers/RecipeCommandHandler.cpp # 菜谱命令处理程序
    src/cli/handlers/AdminCommandHandler.cpp  # 管理员命令处理程序
    # src/api/dll_api.cpp # DLL 接口文件暂时不包含在 CLI 中
    # 注意: CustomLinkedList.h 是仅头文件库，不需要添加到源文件列表
)
message(STATUS "Executable Source files: ${EXECUTABLE_SOURCES}")

# --- 添加可执行文件目标 ---
add_executable(recipe-cli ${EXECUTABLE_SOURCES})
message(STATUS "Added executable target: recipe-cli")

# --- 添加包含目录 (使用 target_include_directories) ---
# 将包含目录明确关联到 recipe-cli 目标
target_include_directories(recipe-cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include # For json.hpp and potentially cxxopts.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src     # For headers within the src directory
)
message(STATUS "Target include paths for recipe-cli added.")

# --- (可选) 平台特定设置 ---
# 对于可执行文件，通常不需要 POSITION_INDEPENDENT_CODE，除非它被用作某种插件
# if(UNIX)
#     set_target_properties(recipe-cli PROPERTIES POSITION_INDEPENDENT_CODE ON)
# endif()

# --- (可选) 链接依赖库 ---
# 如果您的代码依赖其他库，在此处链接
# target_link_libraries(recipe-cli PRIVATE SomeOtherLib::SomeOtherLib)

# --- (可选) 安装规则 ---
# 定义如何安装生成的可执行文件 (如果需要)
# install(TARGETS recipe-cli DESTINATION bin)
# install(FILES include/json.hpp DESTINATION include/nlohmann)
# install(FILES src/domain/recipe/Recipe.h src/logic/recipe/RecipeManager.h ... DESTINATION include/RecipeCliApp)
# --- 启用测试 ---
enable_testing()

# --- 使用 FetchContent 获取并配置 Google Test ---
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG    release-1.12.1 # 或者你希望使用的特定版本/标签
)

# 强制 gtest 使用共享 CRT (MD/MDd) 来匹配项目设置
set(gtest_force_shared_crt ON CACHE BOOL "Force shared CRT for gtest" FORCE)
# 对于 googletest，我们需要显式地将其标记为非独占，以避免在多个目录中获取它时出现问题
# FetchContent_GetProperties(googletest)
# if(NOT googletest_POPULATED) # 只有在尚未填充时才填充
  FetchContent_MakeAvailable(googletest)
# endif()

# FetchContent_MakeAvailable(googletest) 将会:
# 1. 下载 googletest (如果尚未下载)
# 2. 在构建树中添加 googletest 的 CMake 项目 (通常在 _deps 目录下)
# 3. 创建 GTest::gtest, GTest::gtest_main, GTest::gmock, GTest::gmock_main 等目标
#    这些目标会自动处理包含目录和链接依赖。

# find_package(GTest REQUIRED) # 这一行不再需要，因为 FetchContent 会处理

# 如果找到了 GTest (通过 FetchContent)，GTest::gtest, GTest::gtest_main 等目标将可用。
# 我们通常链接到 GTest::gtest_main，它包含了 gtest 和一个 main 函数。
# 或者，如果你的测试文件自己提供了 main 函数，则链接到 GTest::gtest。

# --- 添加测试: TestCustomLinkedList ---
add_executable(TestCustomLinkedList tests/TestCustomLinkedList.cpp)
# 确保测试可以找到项目头文件
target_include_directories(TestCustomLinkedList PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include # For json.hpp etc.
    ${CMAKE_CURRENT_SOURCE_DIR}/src     # For project headers
)
# 链接 Google Test
target_link_libraries(TestCustomLinkedList PRIVATE GTest::gtest_main)
add_test(NAME TestCustomLinkedList COMMAND TestCustomLinkedList)

message(STATUS "Added test: TestCustomLinkedList")
# --- 添加测试: TestRestaurant ---
add_executable(TestRestaurant
    tests/TestRestaurant.cpp
    src/domain/restaurant/Restaurant.cpp
)
# 确保测试可以找到项目头文件
target_include_directories(TestRestaurant PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
# 链接 Google Test
target_link_libraries(TestRestaurant PRIVATE GTest::gtest_main)
add_test(NAME TestRestaurant COMMAND TestRestaurant)

message(STATUS "Added test: TestRestaurant")
# --- 添加测试: TestRestaurantManager ---
add_executable(TestRestaurantManager
    tests/TestRestaurantManager.cpp
    src/logic/restaurant/RestaurantManager.cpp
    src/domain/restaurant/Restaurant.cpp
    src/persistence/JsonRestaurantRepository.cpp
    src/logic/recipe/RecipeManager.cpp
    src/domain/recipe/Recipe.cpp
    src/persistence/JsonRecipeRepository.cpp
)
# 确保测试可以找到项目头文件
target_include_directories(TestRestaurantManager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
# 链接 Google Test
target_link_libraries(TestRestaurantManager PRIVATE GTest::gtest_main)
add_test(NAME TestRestaurantManager COMMAND TestRestaurantManager)

message(STATUS "Added test: TestRestaurantManager")
# --- 添加测试: TestPersistenceRestaurant ---
add_executable(TestPersistenceRestaurant
    tests/TestPersistenceRestaurant.cpp
    src/persistence/PersistenceManager.cpp
    src/logic/user/UserManager.cpp
    src/domain/user/User.cpp
    src/persistence/JsonUserRepository.cpp
    src/logic/recipe/RecipeManager.cpp
    src/domain/recipe/Recipe.cpp
    src/persistence/JsonRecipeRepository.cpp
    src/logic/restaurant/RestaurantManager.cpp
    src/domain/restaurant/Restaurant.cpp
    src/persistence/JsonRestaurantRepository.cpp
)
# 确保测试可以找到项目头文件
target_include_directories(TestPersistenceRestaurant PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
# 链接 Google Test
target_link_libraries(TestPersistenceRestaurant PRIVATE GTest::gtest_main)
add_test(NAME TestPersistenceRestaurant COMMAND TestPersistenceRestaurant)

message(STATUS "Added test: TestPersistenceRestaurant")
# --- 添加测试: TestPersistenceComprehensive ---
add_executable(TestPersistenceComprehensive
    tests/TestPersistenceComprehensive.cpp
    src/persistence/PersistenceManager.cpp
    src/logic/user/UserManager.cpp
    src/domain/user/User.cpp
    src/persistence/JsonUserRepository.cpp
    src/logic/recipe/RecipeManager.cpp
    src/domain/recipe/Recipe.cpp
    src/persistence/JsonRecipeRepository.cpp
    src/logic/restaurant/RestaurantManager.cpp
    src/domain/restaurant/Restaurant.cpp
    src/persistence/JsonRestaurantRepository.cpp
)
# 确保测试可以找到项目头文件
target_include_directories(TestPersistenceComprehensive PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
# 链接 Google Test
target_link_libraries(TestPersistenceComprehensive PRIVATE GTest::gtest_main)
add_test(NAME TestPersistenceComprehensive COMMAND TestPersistenceComprehensive)

message(STATUS "Added test: TestPersistenceComprehensive")

# --- 添加测试: TestRecipe ---
add_executable(TestRecipe
    tests/TestRecipe.cpp
    src/domain/recipe/Recipe.cpp
)
# 确保测试可以找到项目头文件和 GTest 头文件
target_include_directories(TestRecipe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    # ${GTEST_INCLUDE_DIRS} # 不再需要，GTest::gtest_main 会处理包含目录
)
# 链接 Google Test
target_link_libraries(TestRecipe PRIVATE GTest::gtest_main)
add_test(NAME TestRecipe COMMAND TestRecipe)

message(STATUS "Added test: TestRecipe")

# --- 添加测试: TestJsonRecipeRepository ---
add_executable(TestJsonRecipeRepository
    tests/TestJsonRecipeRepository.cpp
    src/persistence/JsonRecipeRepository.cpp
    src/domain/recipe/Recipe.cpp # JsonRecipeRepository 依赖 Recipe
)
# 确保测试可以找到项目头文件和 GTest 头文件
target_include_directories(TestJsonRecipeRepository PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    # ${GTEST_INCLUDE_DIRS} # 不再需要，GTest::gtest_main 会处理包含目录
)
# 链接 Google Test
target_link_libraries(TestJsonRecipeRepository PRIVATE GTest::gtest_main)
add_test(NAME TestJsonRecipeRepository COMMAND TestJsonRecipeRepository)

message(STATUS "Added test: TestJsonRecipeRepository")