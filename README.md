# IntelligentRecipeManagementSystem (RecipeCliApp)

## 一、项目概述与核心价值

IntelligentRecipeManagementSystem (项目代号: RecipeCliApp) 是一个旨在帮助用户高效管理、便捷搜索个人食谱的命令行应用程序。经过近期的重大升级，本项目在原有基础上进行了深度重构和功能增强，致力于为美食爱好者和烹饪学习者提供一个更强大、更稳定、更易用的集中化平台，以数字化方式组织和利用他们的烹饪知识和创意。

**核心价值：**

*   **高效管理**：用户可以轻松添加、编辑、删除和组织自己的食谱，并通过标签系统进行精细化分类。
*   **便捷搜索**：提供基于食谱名称、标签等多种方式的搜索功能，用户可以快速找到所需食谱。
*   **数据健壮性**：通过优化的数据结构和原子化的持久化操作，确保用户数据的安全与一致性。
*   **跨平台支持**：确保在 Linux 和 Windows 平台均能流畅运行。
*   **开放与扩展**：支持通过 JSON 文件批量导入导出食谱数据，方便数据迁移与共享。

## 二、主要特性概览

*   **全面的食谱管理**：支持食谱的增、删、改、查、视图、搜索等操作。
*   **食谱分类与标签**：允许用户为食谱添加自定义标签，方便分类和检索。
*   **批量数据操作**：支持从 JSON 文件批量导入食谱，以及将现有食谱批量导出到 JSON 文件。
*   **优化的命令行界面 (CLI)**：提供简洁高效的命令行操作方式，并有详细的帮助信息。
*   **可靠的数据持久化**：食谱数据以 JSON 格式安全存储在用户特定的配置目录中，采用原子化写入确保数据完整性。
*   **跨平台兼容**：基于 CMake 构建，支持 g++ (Linux) 和 MSVC (Windows) 编译器。
*   **(内部) 餐厅数据管理**：系统内部包含对餐厅数据的管理逻辑和持久化支持 (存储于 `restaurants.json`)，为未来扩展相关功能奠定了基础 (当前版本未提供命令行接口)。

## 三、核心重构与优化措施

本次项目升级包含了以下关键的重构和优化工作，旨在提升代码质量、程序性能和用户体验：

*   **数据结构升级**：
    *   核心食谱存储已从自定义链表 (`CustomLinkedList`) 迁移到 C++ 标准库的 `std::vector`。
    *   **益处**：提高了数据访问效率，简化了内存管理，减少了潜在的错误，并更好地利用了标准库的算法和功能。
*   **持久化机制改进**：
    *   **路径管理**：采用 C++17 `<filesystem>` 库进行跨平台的路径操作和用户配置目录的获取，确保数据文件存储在标准位置（例如 Linux 下的 `~/.config/IntelligentRecipeManagementSystem` 或 Windows 下的 `%APPDATA%\IntelligentRecipeManagementSystem`）。
    *   **原子化保存**：在保存食谱数据到文件时，采用先写入临时文件再重命名的策略，以防止在写入过程中发生意外（如程序崩溃或磁盘空间不足）导致数据损坏或丢失。
*   **代码质量提升**：
    *   **`UserManager` 清理**：对 `UserManager` 类进行了审查和简化，移除了不再需要的功能（如用户注册登录相关逻辑），使其更专注于核心职责。
    *   **错误处理**：增强了整个应用程序的错误处理机制，对文件操作、用户输入等方面提供了更明确的错误提示和更健壮的异常处理。
    *   **输入校验**：加强了对用户输入的校验，防止无效数据导致程序异常。

这些重构和优化为项目的长期维护和未来功能扩展奠定了坚实的基础。

## 四、项目结构

本项目采用模块化的代码结构，主要分为以下几个核心部分：

*   **Domain (`src/domain`)**: 定义了核心业务对象，如 `Recipe` (食谱) 和 `Restaurant` (餐厅)，以及它们的属性和基本行为。
*   **Logic (`src/logic`)**: 包含业务逻辑处理层，如 `RecipeManager` 和 `RestaurantManager`，负责协调领域对象和持久化层之间的操作。
*   **Persistence (`src/persistence`)**: 实现了数据持久化功能，如 `JsonRecipeRepository` 和 `JsonRestaurantRepository`，负责将领域对象以 JSON 格式存储到文件系统，并从中加载。
*   **CLI (`src/cli`)**: 包含了命令行界面的实现，包括参数解析 (使用 `cxxopts`)、命令处理 (`RecipeCommandHandler`) 以及相关的工具函数。
*   **Include (`include`)**: 存放项目依赖的第三方头文件库，如 `cxxopts.hpp` 和 `json.hpp`。

这种分层结构有助于保持代码的组织性、可维护性和可测试性。

## 五、环境准备与安装部署

本项目基于 C++17 和 CMake 构建，旨在提供良好的跨平台兼容性，支持在 Linux (使用 g++) 和 Windows (使用 MSVC) 环境下编译和运行。

**先决条件：**

*   **操作系统**：
    *   Linux (推荐 Ubuntu, Fedora, Debian 等主流发行版)
    *   Windows (Windows 10 或更高版本)
*   **编译器**：
    *   Linux: g++ (建议版本 9.0 或更高，以确保对 C++17 `<filesystem>` 的完整支持)
    *   Windows: MSVC (Visual Studio 2019 或更高版本，同样需要 C++17 支持)
*   **构建工具**：CMake (建议版本 3.15 或更高)
*   **Git**：用于克隆项目仓库。
*   **第三方库** (已包含在项目 `include` 目录中，无需额外安装)：
    *   [cxxopts](https://github.com/jarro2783/cxxopts): 用于命令行参数解析。
    *   [nlohmann/json](https://github.com/nlohmann/json): 用于 JSON 数据的序列化和反序列化。

**安装步骤：**

1.  **克隆项目仓库**：
    ```bash
    git clone https://github.com/ivekasy/IntelligentRecipeManagementSystem.git
    cd IntelligentRecipeManagementSystem
    ```

2.  **创建构建目录并运行 CMake**：
    ```bash
    mkdir build
    cd build
    cmake ..
    ```
    *   **Windows (MSVC 用户)**: 如果您安装了多个 Visual Studio 版本或需要指定特定的生成器，可以使用 `-G` 选项，例如：
        ```bash
        cmake .. -G "Visual Studio 17 2022" -A x64
        ```
    *   如果 CMake 提示找不到编译器或依赖，请确保已正确安装相应的编译器和开发库。

3.  **编译项目**：
    ```bash
    cmake --build . --config Release
    ```
    *   我们推荐使用 `Release` 配置进行编译以获得优化后的性能。如果需要调试，可以使用 `--config Debug`。
    *   编译成功后，可执行文件通常位于：
        *   Linux: `build/recipe-cli`
        *   Windows: `build\Release\recipe-cli.exe` (或 `build\Debug\recipe-cli.exe`)

4.  **运行程序**：
    建议进入可执行文件所在的目录执行，或者将该目录添加到系统的 PATH 环境变量中。
    *   **Linux (在 `build` 目录中):**
        ```bash
        ./recipe-cli
        ```
    *   **Windows (例如在 `build\Release` 目录中):**
        ```bash
        .\recipe-cli.exe
        ```
    或者，您也可以从项目根目录使用相对路径执行，例如：
    *   Linux: `./build/recipe-cli`
    *   Windows: `.\build\Release\recipe-cli.exe`

    运行 `recipe-cli --help` (或对应平台的命令) 可以查看所有可用命令。

5.  **数据文件存储**：
    *   程序首次运行时，会在用户配置目录下自动创建 `IntelligentRecipeManagementSystem` 文件夹，并在其中存储 `recipes.json` 数据文件。
    *   **Linux**: 通常是 `~/.config/IntelligentRecipeManagementSystem/recipes.json`
    *   **Windows**: 通常是 `%APPDATA%\IntelligentRecipeManagementSystem\recipes.json` (例如 `C:\Users\<YourUser>\AppData\Roaming\IntelligentRecipeManagementSystem\recipes.json`)

## 六、快速入门指南

以下示例引导您快速体验核心功能。请确保您已按照上一节的说明成功编译并可以运行程序。为简洁起见，以下示例将使用 `recipe-cli` (Linux) 或 `recipe-cli.exe` (Windows) 作为命令的代表。

1.  **启动程序并查看帮助**：
    ```bash
    recipe-cli --help
    ```

2.  **添加一个新食谱**:
    执行以下命令后，程序会引导您输入食谱的详细信息（名称、食材、步骤、烹饪时间、难度、菜系、标签等）。
    ```bash
    recipe-cli --recipe-add
    ```

3.  **查看所有食谱**:
    ```bash
    recipe-cli --recipe-list
    ```

4.  **为食谱添加标签**:
    假设您有一个 ID 为 101 的食谱，想要为其添加 "晚餐" 和 "快捷" 标签，可以使用以下命令更新食谱：
    ```bash
    recipe-cli --recipe-update 101 --tags "晚餐,快捷"
    ```
    *注意：多个标签之间用英文逗号分隔。*

5.  **按标签搜索食谱**:
    搜索包含 "快捷" 标签的所有食谱：
    ```bash
    recipe-cli --recipe-search --tags "快捷"
    ```

通过以上步骤，您应该已经成功添加、标记并查看了一个食谱。

## 七、详细功能使用教程

所有命令均以 `recipe-cli` (Linux) 或 `recipe-cli.exe` (Windows) 开头，并假设您已位于可执行文件所在目录或已将程序路径添加到系统 PATH。

### 1. 通用命令

*   **显示帮助信息**:
    ```bash
    recipe-cli --help
    recipe-cli -h
    ```
*   **显示版本信息**:
    ```bash
    recipe-cli --version
    recipe-cli -v
    ```
*   **启用详细输出模式 (Verbose Mode)**:
    此选项可以提供更详细的程序执行信息，有助于调试。
    ```bash
    recipe-cli --verbose <其他命令和参数>
    ```
    例如: `recipe-cli --verbose --recipe-list`

### 2. 食谱管理 (核心功能)

*   **添加新食谱 (`--recipe-add`)**:
    *   执行命令后，程序会通过一系列交互式提示引导您输入食谱的详细信息，包括：
        *   食谱名称 (必填)
        *   食材 (名称和用量，可添加多个，按提示操作)
        *   制作步骤 (可添加多个，按提示操作)
        *   烹饪时间 (分钟，可选)
        *   准备时间 (分钟，可选)
        *   难度 (Easy, Medium, Hard，可选)
        *   菜系 (例如：川菜、粤菜，可选)
        *   标签 (多个标签用英文逗号 `,` 分隔，例如 "家常菜,快捷,晚餐"，可选)。可以使用 `--tags "标签1,标签2"` 选项在添加食谱时指定标签。
        *   营养信息 (可选)
        *   图片 URL (可选)
    *   示例: `recipe-cli --recipe-add --tags "家常菜,快捷"`

*   **查看所有食谱 (`--recipe-list`)**:
    *   列出所有已保存的食谱及其基本信息（ID、名称）。
    *   示例: `recipe-cli --recipe-list`

*   **查看食谱详情 (`--recipe-view <RECIPE_ID>`)**:
    *   `RECIPE_ID`: 要查看的食谱的数字 ID。可以通过 `--recipe-list` 获取。
    *   显示指定食谱的所有详细信息，包括食材、步骤、标签等。
    *   示例: `recipe-cli --recipe-view 101`

*   **搜索食谱 (`--recipe-search [=查询内容 (可选)(=)]`)**:
    *   此命令可以按名称、标签或组合搜索食谱。
    *   **按名称搜索**:
        *   `QUERY`: 要搜索的食谱名称关键词。如果关键词包含空格，请用双引号括起来。
        *   示例: `recipe-cli --recipe-search "红烧"`
        *   示例: `recipe-cli --recipe-search "Chicken Soup"`
    *   **按单个标签搜索**:
        *   `--tag 标签名`:  用于按单个标签过滤。
        *   示例: `recipe-cli --recipe-search --tag "素食"`
    *   **按多个标签搜索 (AND 逻辑)**:
        *   `--tags 逗号分隔的标签列表`: 用于 AND 匹配多个标签。
        *   格式: "标签1,标签2,标签3"
        *   示例: `recipe-cli --recipe-search --tags "晚餐,快捷"`
    *   **组合搜索 (名称和标签)**:
        *   可以组合名称和标签进行搜索。
        *   示例: `recipe-cli --recipe-search "汤" --tag "冬季"`

*   **更新食谱 (`--recipe-update <RECIPE_ID>`)**:
    *   `RECIPE_ID`: 要更新的食谱的数字 ID。
    *   执行命令后，程序会显示当前食谱信息，并引导您交互式地更新各个字段。对于不想修改的字段，可以直接按回车跳过。
    *   可以使用 `--tags "新标签1,新标签2"` 选项更新标签 (将替换所有旧标签)。
    *   示例: `recipe-cli --recipe-update 101 --tags "健康,午餐"`

*   **删除食谱 (`--recipe-delete <RECIPE_ID>`)**:
    *   `RECIPE_ID`: 要删除的食谱的数字 ID。
    *   删除前会有确认提示，输入 `y` 或 `yes` 确认删除。
    *   示例: `recipe-cli --recipe-delete 101`

### 3. 标签管理

标签系统帮助您更好地组织和检索食谱。
(此部分内容已整合到 `--recipe-add`, `--recipe-update`, 和 `--recipe-search` 命令描述中。)


### 4. 批量导入与导出

(此功能当前版本未通过命令行提供。)


*(请注意：上述命令和参数是根据当前功能设计的。建议始终运行 `recipe-cli --help` 以获取最准确和完整的命令列表及其用法。)*

## 八、配置选项说明

*   **数据存储位置**：
    *   食谱数据 (`recipes.json`) 默认存储在用户特定的配置目录中。这确保了不同用户的食谱数据相互隔离，并且符合操作系统的标准实践。
        *   **Linux**: `~/.config/IntelligentRecipeManagementSystem/recipes.json`
        *   **Windows**: `%APPDATA%\IntelligentRecipeManagementSystem\recipes.json` (例如: `C:\Users\<YourUserName>\AppData\Roaming\IntelligentRecipeManagementSystem\recipes.json`)
    *   程序在首次运行时会自动创建此目录（如果尚不存在）。
    *   除了 `recipes.json`，程序还会在此目录中存储 `restaurants.json` 文件，用于保存餐厅数据。
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
        *   **Linux**: `~/.config/IntelligentRecipeManagementSystem/`
        *   **Windows**: `%APPDATA%\IntelligentRecipeManagementSystem\`
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
        *   **标签管理**: 标签的添加、更新和搜索功能已整合入 `--recipe-add`、`--recipe-update` 和 `--recipe-search` 命令中，通过 `--tags` 和 `--tag` 选项实现。移除了独立的 `--recipe-add-tags`, `--recipe-remove-tags`, `--recipe-search-by-tags`, `--recipe-list-tags` 命令。
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