# Intelligent Recipe Management System v0.2.0

Intelligent Recipe Management System 是一个基于命令行的C++应用程序，旨在帮助用户高效地管理菜谱和相关的餐馆信息。

## 项目概述

本项目提供了一个强大的命令行界面(CLI)，用于对菜谱进行全面的增删改查操作，包括按名称、食材、菜系和标签进行搜索。同时，系统也支持对餐馆信息的管理。数据通过JSON文件进行持久化存储。

当前版本 **v0.2.0** 主要集中在代码库的重构和改进，为未来的功能扩展奠定了坚实的基础。

## 功能特性

*   **菜谱管理**:
    *   添加新菜谱（包括名称、食材、步骤、烹饪时间、难度、菜系、标签、可选的营养信息和图片链接）。
    *   查看所有菜谱列表。
    *   根据ID查看单个菜谱的详细信息。
    *   按名称搜索菜谱（支持精确匹配和部分匹配）。
    *   按菜系搜索菜谱。
    *   按单个或多个标签搜索菜谱（支持AND匹配）。
    *   按食材搜索菜谱（支持匹配所有或任一食材）。
    *   更新现有菜谱信息。
    *   删除菜谱。
*   **餐馆管理** (核心逻辑已实现，CLI命令正在逐步集成):
    *   添加新餐馆（包括名称、地址、联系方式、营业时间、特色菜谱ID）。
    *   查看、更新、删除餐馆信息。
    *   管理餐馆的特色菜谱。
*   **数据持久化**:
    *   菜谱和餐馆数据存储在用户配置目录下的JSON文件中。
    *   采用原子化写入策略，增强数据保存的安全性。
*   **命令行界面**:
    *   通过 `cxxopts` 库提供用户友好的命令行参数解析。
    *   支持详细输出模式 (`--verbose`)。

## 技术栈与架构

*   **主要语言**: C++ (推荐使用支持 C++17 及以上标准的编译器)
*   **核心库**:
    *   [nlohmann/json](https://github.com/nlohmann/json): 用于JSON序列化与反序列化。
    *   [cxxopts](https://github.com/jarro2783/cxxopts): 用于命令行参数解析。
*   **项目架构**:
    *   **分层架构**:
        1.  **CLI层**: 处理用户输入，调用应用逻辑。
        2.  **逻辑层 (Managers)**: `RecipeManager`, `RestaurantManager` - 封装业务逻辑和流程协调。
        3.  **领域层 (Domain)**: `Recipe`, `Restaurant` (实体), `RecipeRepository`, `RestaurantRepository` (仓库接口) - 定义核心业务对象和数据访问契约。
        4.  **持久化层 (Persistence)**: `JsonRecipeRepository`, `JsonRestaurantRepository` (仓库实现) - 负责将领域对象持久化到JSON文件。通过模板基类 `JsonRepositoryBase` 实现通用逻辑。
    *   **依赖注入**: Manager 类通过构造函数注入其依赖的 Repository 接口，实现了与具体持久化实现的解耦。

## 安装与构建

### 依赖项

1.  **C++编译器**: 支持 C++17 标准 (例如 GCC 7+, Clang 5+, MSVC 2017+)。
2.  **CMake**: 版本 3.10 或更高。
3.  **第三方库**:
    *   `nlohmann/json.hpp`: 单头文件JSON库。项目已包含此库，通常位于 `include/external/` 或由CMake通过 `FetchContent` (如果配置) 管理。确保您的构建系统可以找到它。
    *   `cxxopts.hpp`: 单头文件命令行参数解析库。项目已包含此库，通常位于 `include/external/` 或由CMake通过 `FetchContent` (如果配置) 管理。确保您的构建系统可以找到它。

### 构建步骤

```bash
# 1. 克隆仓库 (如果尚未克隆)
# git clone <repository_url>
# cd IntelligentRecipeManagementSystem

# 2. 创建构建目录并进入
mkdir build
cd build

# 3. 运行 CMake 配置项目
cmake ..

# 4. 构建项目
# 在 Linux/macOS 上:
make
# 在 Windows 上 (使用 MSVC，根据您的构建工具可能有所不同):
# cmake --build . --config Release  # 或者 --config Debug
# 或者直接在 Visual Studio 中打开生成的解决方案并构建 (选择 Release 或 Debug 配置)。

# 5. 可执行文件
# 构建成功后，可执行文件 `recipe-cli` (或 `recipe-cli.exe`) 将位于 `build/` 目录 (对于单配置生成器如 Makefiles)
# 或 `build/Release/` 或 `build/Debug/` 目录 (对于多配置生成器如 Visual Studio)。
```

## 使用指南

构建完成后，您可以在 `build/` 目录下找到可执行文件 `recipe-cli`。

#### 基本命令

*   显示帮助信息:
    ```bash
    ./recipe-cli --help
    ```
*   显示版本号:
    ```bash
    ./recipe-cli --version
    ```
*   启用详细输出:
    ```bash
    ./recipe-cli <command> --verbose
    ```

#### 菜谱管理命令示例

*   **添加菜谱 (交互式)**:
    ```bash
    ./recipe-cli --recipe-add
    ```
    程序将引导您输入菜谱的各项信息。
*   **添加菜谱 (通过命令行参数指定标签)**:
    ```bash
    ./recipe-cli --recipe-add --tags "家常菜,快捷,晚餐"
    ```
*   **列出所有菜谱**:
    ```bash
    ./recipe-cli --recipe-list
    ```
*   **查看菜谱详情**:
    ```bash
    ./recipe-cli --recipe-view 101 
    ```
    (将 `101` 替换为实际的菜谱ID)
*   **搜索菜谱**:
    *   按名称 (部分匹配): `./recipe-cli --recipe-search "鸡丁"`
    *   按单个标签: `./recipe-cli --recipe-search --tag "素食"`
    *   按多个标签 (AND匹配): `./recipe-cli --recipe-search --tags "晚餐,快捷"`
    *   组合名称和标签: `./recipe-cli --recipe-search "汤" --tag "冬季"`
*   **更新菜谱 (交互式)**:
    ```bash
    ./recipe-cli --recipe-update 101
    ```
    (将 `101` 替换为实际的菜谱ID) 程序将引导您更新信息。
*   **更新菜谱 (通过命令行参数更新标签)**:
    ```bash
    ./recipe-cli --recipe-update 101 --tags "健康,午餐,低卡"
    ```
    (这将替换菜谱101的所有现有标签)
*   **删除菜谱**:
    ```bash
    ./recipe-cli --recipe-delete 101
    ```
    (将 `101` 替换为实际的菜谱ID)

#### 餐馆管理命令示例
(注意: 餐馆管理的CLI命令接口正在开发和集成中，以下为预期功能示例，具体可用命令请参照 `--help`。)
*   `./recipe-cli --restaurant-add` (预期)
*   `./recipe-cli --restaurant-list` (预期)

#### 数据文件位置
菜谱数据 (`recipes.json`) 和餐馆数据 (`restaurants.json`) 默认存储在用户特定的配置目录中。详细路径及备用存储位置，请参见本文档的 **“八、配置选项说明”** 部分。

## v0.2.0 重构与优化摘要

此版本主要关注于提升代码质量、可维护性和为未来功能奠定基础。

*   **持久化层重构**:
    *   引入了模板基类 `JsonRepositoryBase<T>`，统一了 `JsonRecipeRepository` 和 `JsonRestaurantRepository` 中针对JSON文件的通用数据读写、ID管理和核心CRUD操作的逻辑。
    *   此项重构显著减少了持久化模块中的代码重复，提高了代码的可维护性和扩展性。
*   **代码清理与现代化**:
    *   移除了项目中不再使用的自定义数据结构 `CustomLinkedList`，全面转向使用C++标准库容器 (如 `std::vector`)。
    *   清理了代码库中与 `CustomLinkedList` 相关的过时注释和引用。
*   **API (`dll_api.cpp`) 改进**:
    *   修正了 `dll_api.cpp` 中全局 `RecipeManager` 实例的初始化方式，采用了指针和显式的初始化/销毁函数，并确保了正确的仓库依赖注入。
    *   改进了 `get_recipe_by_id_json` API函数对 `std::optional` 返回类型的处理。
    *   更新了 `dll_api.cpp` 中与已移除的 `CustomLinkedList` 相关的过时注释。
*   **错误修复**:
    *   修正了 `Restaurant.cpp` 中可能存在的重复 `to_json` 定义和命名空间关闭错误。
*   **代码分析**: 对整个代码库进行了初步分析，识别了包括错误处理一致性、日志记录、`includePath` 配置、潜在代码重复以及测试覆盖率不足等多个待改进领域。

## 已知问题与未来工作

*   **`includePath` 配置问题**: 当前在某些开发环境（如VS Code的IntelliSense）下可能会遇到头文件找不到的错误 (例如 `json.hpp`, `cxxopts.hpp`)。这通常是由于编译器的包含路径未正确配置。**建议用户检查并配置其构建环境/IDE的 `includePath` 设置，确保项目根目录下的 `include/` 和 `src/` 目录被正确识别。**
*   **错误处理与日志**: 项目当前的错误处理（异常与返回码混用）和日志记录（主要使用 `std::cout`/`std::cerr`）机制有待进一步统一和完善。计划引入更规范的日志框架和一致的异常处理策略。
*   **测试覆盖率**: 自动化测试覆盖率较低。未来的迭代将重点增加单元测试和集成测试，以确保代码质量和重构的安全性。
*   **餐馆CLI命令**: 餐馆管理相关的命令行接口尚未完全集成到 `main.cpp` 中，后续版本将逐步完善。
*   **新功能**: (为后续任务预留) 计划添加用户账户管理、更高级的搜索过滤选项、以及可能的图形用户界面(GUI)等功能。
*   **删除 `CustomLinkedList.h`**: 在确认所有依赖已清理后，[`src/core/CustomLinkedList.h`](src/core/CustomLinkedList.h) 文件将从项目中移除。

## 八、配置选项说明

*   **数据存储位置**：
    *   菜谱数据 (`recipes.json`) 和餐馆数据 (`restaurants.json`) 默认存储在用户特定的配置目录的 `data` 子目录下。这确保了不同用户的食谱数据相互隔离，并且符合操作系统的标准实践。
        *   **Linux/macOS**: `~/.config/IntelligentRecipeManagementSystem/data/`
        *   **Windows**: `%APPDATA%\IntelligentRecipeManagementSystem\data\` (例如: `C:\Users\<YourUserName>\AppData\Roaming\IntelligentRecipeManagementSystem\data\`)
    *   程序在首次运行时会自动创建这些目录（如果尚不存在）。
    *   **备用位置**: 如果相应的用户配置目录环境变量 (`APPDATA` 或 `HOME`) 未设置，程序会尝试在当前工作目录下创建 `.IntelligentRecipeManagementSystem_UserConfig/data/` 作为数据存储的回退位置。
*   **配置文件**：
    *   当前版本不使用外部配置文件来修改程序行为或数据路径。所有配置均通过代码内部定义或命令行参数控制。

## 九、API 参考文档

目前项目主要通过命令行界面进行交互。如果未来计划提供 HTTP API 或库 API，将在此处详细说明。

## 十、故障排除与常见问题解答 (FAQ)

*   **Q1: 编译失败，提示找不到 `CMakeLists.txt`?**
    *   A: 请确保您在执行 `cmake ..` 命令时，当前目录是 `build` 目录，并且 `build` 目录与 `CMakeLists.txt` 文件所在的项目的根目录是同级。

*   **Q2: 运行程序时提示“命令未找到”或类似错误 (例如 `./recipe-cli: No such file or directory` on Linux, or `'recipe-cli.exe' is not recognized...` on Windows)?**
    *   A:
        1.  **检查编译是否成功**：确保可执行文件 (`recipe-cli` 或 `recipe-cli.exe`) 已成功生成在预期的 `build` 目录（例如 `build/` 或 `build/Release/`）中。
        2.  **检查执行路径**：
            *   如果您在可执行文件所在的目录中，Linux 用户使用 `./recipe-cli`，Windows 用户使用 `.\recipe-cli.exe`。
            *   如果您在项目根目录中，Linux 用户使用 `./build/recipe-cli`，Windows 用户使用 `.\build\Release\recipe-cli.exe` (路径可能因您的构建配置而异)。
        3.  **PATH 环境变量 (可选)**: 如果希望在任何目录下都能直接运行 `recipe-cli`，可以将包含可执行文件的目录添加到系统的 PATH 环境变量中。

*   **Q3: 如何查看所有可用的命令？**
    *   A: 运行 `recipe-cli --help` 或 `recipe-cli -h`。

*   **Q4: 添加食谱时，如果名称、步骤或标签包含空格怎么办？**
    *   A:
        *   对于 `--recipe-add` 命令，所有信息都是通过交互式提示输入的，程序会处理包含空格的输入。
        *   对于其他接受参数的命令（如 `--recipe-search "Chicken Soup"` 或使用 `--tags` 选项时），如果参数值本身包含空格或需要传递多个用逗号分隔的值，请将整个参数值用双引号 `"` 括起来。

*   **Q5: 批量导入食谱时，JSON 文件应该是什么格式？**
    *   A: 当前版本未通过命令行提供批量导入功能。如果未来添加此功能，JSON 文件应为一个包含食谱对象的数组。一个食谱对象大致包含以下字段：`id` (导入时程序会自动处理或忽略), `name`, `ingredients` (对象数组), `instructions` (字符串数组), `cookingTime`, `preparationTime`, `difficulty`, `cuisine`, `tags` (字符串数组), `nutritionInfo`, `imageUrl`。

*   **Q6: 为什么我的食谱数据没有保存在程序运行的目录？**
    *   A: 为了更好地管理用户数据并遵循操作系统规范，程序现在将 `recipes.json` 和 `restaurants.json` 文件存储在用户特定的配置目录中：
        *   **Linux/macOS**: `~/.config/IntelligentRecipeManagementSystem/data/`
        *   **Windows**: `%APPDATA%\IntelligentRecipeManagementSystem\data\`
        这是预期的行为，有助于保持项目目录的整洁，并支持多用户环境（尽管当前版本主要面向单用户）。

*   **Q7: 在 Windows 上使用 CMake 时遇到 "generator ... could not be found" 错误。**
    *   A: 这通常意味着 CMake 无法找到您希望使用的 Visual Studio 版本。请确保已安装 Visual Studio (带有 C++桌面开发工作负载)。您可以尝试在 `cmake ..` 命令后明确指定生成器，例如 `cmake .. -G "Visual Studio 17 2022" -A x64` (版本号根据您的安装调整)。

*   **Q8: 如何更新到最新版本的程序？**
    *   A:
        1.  进入您的本地项目目录。
        2.  拉取最新的代码：`git pull origin main` (或者您当前跟踪的主分支名称)。
        3.  重新编译项目：进入 `build` 目录，然后运行 `cmake ..` (如果 `CMakeLists.txt` 有更改) 和 `cmake --build . --config Release`。

## 十一、参与贡献指南

我们欢迎并感谢所有对本项目的贡献！

**贡献流程：**

1.  **Fork 本仓库** 到您自己的 GitHub 账户。
2.  **Clone 您 fork 的仓库** 到本地：`git clone https://github.com/YOUR_USERNAME/IntelligentRecipeManagementSystem.git`
3.  **创建新的特性分支**：`git checkout -b feature/your-feature-name` 或 `bugfix/issue-number`。
4.  **进行代码修改和开发**。请确保您的代码符合项目的编码规范（如果已定义）。
5.  **提交您的更改**：`git commit -m "feat: Add some amazing feature"` (请遵循 [Conventional Commits](https://www.conventionalcommits.org/) 规范更佳)。
6.  **将您的分支推送到 GitHub**：`git push origin feature/your-feature-name`。
7.  **创建 Pull Request (PR)**：从您 fork 的仓库的特性分支提交一个 PR 到主项目的 `main` 或 `develop` 分支。请在 PR 中清晰描述您的更改内容和目的。

**报告 BUG：**

如果您发现了 BUG，请通过 GitHub Issues 提交详细的 BUG 报告，包括复现步骤、期望行为和实际行为。

**提交功能建议：**

欢迎通过 GitHub Issues 提交您的功能建议和想法。

## 十二、版本历史与更新日志

*   **v0.2.0 (2025-05-10)**
    *   **重大重构与功能增强**:
        *   **数据结构升级**: 核心食谱存储从 `CustomLinkedList` 迁移到 `std::vector`，提升性能和稳定性。
        *   **持久化改进**:
            *   采用 C++17 `<filesystem>` 实现跨平台的路径管理，数据文件 (`recipes.json`, `restaurants.json`) 存储在用户配置目录 (Linux: `~/.config/IntelligentRecipeManagementSystem`, Windows: `%APPDATA%\IntelligentRecipeManagementSystem`)。
            *   实现原子化文件保存，防止数据损坏。
        *   **代码质量**: 清理 `UserManager`，增强错误处理和输入校验。
        *   **项目结构**: 引入更清晰的模块化分层 (Domain, Logic, Persistence, CLI)。
    *   **新功能与调整**:
        *   **模块化 CLI 处理程序**: CLI 处理程序已模块化到 `cli/recipe`, `cli/restaurant`, 和 `cli/user` 目录中。
        *   **标签管理**: 标签的添加、更新和搜索功能已整合入 `--recipe-add`、`--recipe-update` 和 `--recipe-search` 命令中，通过 `--tags` 和 `--tag` 选项实现。移除了独立的 `--recipe-add-tags`, `--recipe-remove-tags`, `--recipe-search-by-tags`, `--recipe-list-tags` 命令。
        *   **依赖注入**: 使用依赖注入来管理 `RecipeManager` 和 `RestaurantManager` 类中的依赖关系。
        *   **错误处理**: 实现了自定义异常类，以提供更具信息性的错误消息。
        *   **日志记录**: 添加了基本的日志记录功能，以记录应用程序事件和错误。
        *   **单元测试**: 为 `RecipeManager` 类添加了单元测试。
        *   **批量导入/导出**: 相关的命令行接口 (`--recipe-import`, `--recipe-export`) 当前版本未提供。
    *   **构建与兼容性**:
        *   优化 [`CMakeLists.txt`](CMakeLists.txt:0) 以更好地支持 g++ (Linux) 和 MSVC (Windows)。
        *   明确了对 C++17 的依赖。
        *   明确了对 `cxxopts` 和 `nlohmann/json` 库的依赖 (已包含在项目中)。
    *   **文档**:
        *   全面更新 [`README.md`](README.md:0) 以反映所有更改、新功能、项目结构和使用说明。
        *   更新了安装和构建指南，使其对 Linux 和 Windows 更清晰。
*   **v0.1.1 (2025-05-09)**
    *   代码清理：移除了未使用的 `UserCommandHandler` 类及其相关引用和代码。
    *   命令行界面：将 `--help` 输出中的选项描述中文化。
    *   Bug修复：
        *   修复了从 `recipes.json` 加载菜谱时因 `cuisine`, `cookingTime`, `preparationTime` 字段名不匹配以及 `difficulty` 字段缺失导致加载失败的问题。
        *   修复了 `admin-user-update` 命令在更新用户密码时，若密码输入为空（不修改密码）则程序意外退出的问题。
    *   功能调整：根据用户请求，在帮助信息和程序执行逻辑中注释掉了所有管理员特定命令 (`--admin-user-list`, `--admin-user-create`, `--admin-user-update`, `--admin-user-delete`)。
    *   文档：全面更新了 README.md，移除了已失效功能（如用户注册登录、管理员命令）的描述和示例，确保文档与程序当前功能一致。
*   **v0.1.0 (2025-05-09)**
    *   初始版本发布。
    *   实现基于命令行的食谱管理 (交互式添加、列表查看、ID查看、名称搜索、更新、删除)。
    *   数据通过 JSON 文件持久化 (存储在程序运行目录)。
    *   使用 cxxopts进行命令行参数解析。
    *   包含基本的 CMake 构建系统和 Google Test 单元测试框架。

## 十三、许可证信息

本项目采用 **MIT 许可证**。

```
MIT License

Copyright (c) 2025 ivekasy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
*(建议在项目根目录创建 `LICENSE` 文件并包含以上内容。)*

## 十四、联系方式与支持渠道

*   **项目维护者邮箱**：simazhangyu@gmail.com (来自原 README)
*   **GitHub Issues**：如果您有任何问题、BUG报告或功能建议，请通过项目仓库的 [Issues](https://github.com/ivekasy/IntelligentRecipeManagementSystem/issues) 页面提交。

---

感谢您使用 IntelligentRecipeManagementSystem (RecipeCliApp)！