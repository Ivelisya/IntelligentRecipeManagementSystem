# IntelligentRecipeManagementSystem (RecipeCliApp)

## 一、项目概述与核心价值

IntelligentRecipeManagementSystem (项目代号: RecipeCliApp) 是一个旨在帮助用户高效管理、便捷搜索个人食谱的命令行应用程序。本项目致力于为美食爱好者和烹饪学习者提供一个集中化的平台，以数字化方式组织和利用他们的烹饪知识和创意。

**核心价值：**

*   **高效管理**：用户可以轻松添加、编辑、删除和组织自己的食谱，告别散乱的纸质笔记或多平台存储的烦恼。
*   **便捷搜索**：提供基于食谱名称的搜索功能，用户可以快速找到所需食谱。
*   **用户账户**：支持用户注册登录，保障个人食谱数据的安全。

## 二、主要特性概览

*   **食谱管理**：全面的食谱增、删、改、查功能。
*   **命令行界面 (CLI)**：提供简洁高效的命令行操作方式。
*   **数据持久化**：食谱和用户信息将以 JSON 格式安全存储在本地文件 (`recipes.json`, `users.json`, `restaurants.json`)。

## 三、环境准备与安装部署

本项目主要基于 C++ 和 CMake 构建，推荐在 Linux 环境下使用 g++ 编译器进行编译和运行。

**先决条件：**

*   **操作系统**：Linux (推荐 Ubuntu, Fedora, Debian 等)。Windows 环境下使用 MSVC 构建理论上可行（CMakeLists.txt 中包含 MSVC 相关设置），但主要测试和推荐环境为 Linux。
*   **编译器**：g++ (建议版本 7.0 或更高，支持 C++17)
*   **构建工具**：CMake (建议版本 3.10 或更高)
*   **Git**：用于克隆项目仓库。

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
    *如果 CMake 提示找不到编译器或依赖，请确保已正确安装 g++ 和其他必要开发库。*

3.  **编译项目**：
    ```bash
    cmake --build .
    ```
    *编译成功后，可执行文件在 `build` 目录内生成。具体路径和名称可能因您的构建系统和配置而异（例如，Linux 下通常是 `build/recipe-cli`，Windows MSVC 下可能是 `build\Debug\recipe-cli.exe` 或 `build\Release\recipe-cli.exe`）。*

4.  **运行程序**：
    建议进入可执行文件所在的目录执行。
    *   **Linux (在 `build` 目录中):**
        ```bash
        ./recipe-cli
        ```
    *   **Windows (例如在 `build\Debug` 目录中):**
        ```bash
        .\recipe-cli.exe
        ```
    或者，您也可以从项目根目录使用相对路径执行，例如：
    *   Linux: `./build/recipe-cli`
    *   Windows: `.\build\Debug\recipe-cli.exe`

    运行 `./recipe-cli --help` (或对应平台的命令) 可以查看所有可用命令。

## 四、快速入门指南

以下示例引导您快速体验核心功能。请确保您在可执行文件所在的目录执行这些命令，或者使用正确的相对/绝对路径调用可执行文件 (参考上一节“运行程序”的说明)。为简洁起见，以下示例将使用 `./recipe-cli` (Linux) 或 `.\recipe-cli.exe` (Windows) 作为在可执行文件目录中运行的代表。

1.  **启动程序并查看帮助**：
    ```bash
    ./recipe-cli --help
    ```

2.  **添加一个新食谱**:
    执行以下命令后，程序会引导您输入食谱的详细信息（名称、食材、步骤、烹饪时间、难度、菜系等）。
    ```bash
    ./recipe-cli --recipe-add
    ```

3.  **查看所有食谱**:
    ```bash
    ./recipe-cli --recipe-list
    ```
通过以上步骤，您应该已经成功添加并查看了一个食谱。


## 五、详细功能使用教程

所有命令均以 `recipe-cli` (Linux) 或 `recipe-cli.exe` (Windows) 开头，并假设您已位于可执行文件所在目录。

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
*   **启用详细输出模式**:
    ```bash
    recipe-cli --verbose <其他命令和参数>
    ```

### 2. 食谱管理

*   **添加新食谱 (`--recipe-add`)**:
    *   执行命令后，程序会通过一系列提示引导您输入食谱的详细信息：
        *   食谱名称
        *   食材 (名称和用量，可添加多个)
        *   制作步骤 (可添加多个)
        *   烹饪时间 (分钟)
        *   难度 (Easy, Medium, Hard)
        *   菜系
        *   营养信息 (可选)
        *   图片 URL (可选)
    *   示例: `recipe-cli --recipe-add`
*   **查看所有食谱 (`--recipe-list`)**:
    *   示例: `recipe-cli --recipe-list`
*   **查看食谱详情 (`--recipe-view <RECIPE_ID>`)**:
    *   `RECIPE_ID`: 食谱的数字 ID。
    *   示例: `recipe-cli --recipe-view 101`
*   **按名称搜索食谱 (`--recipe-search <QUERY>`)**:
    *   `QUERY`: 要搜索的食谱名称关键词。
    *   示例: `recipe-cli --recipe-search "番茄炒蛋"`
*   **更新食谱 (`--recipe-update <RECIPE_ID>`)**:
    *   `RECIPE_ID`: 要更新的食谱的数字 ID。
    *   执行命令后，程序会显示当前食谱信息，并引导您交互式地更新各个字段。
    *   示例: `recipe-cli --recipe-update 101`
*   **删除食谱 (`--recipe-delete <RECIPE_ID>`)**:
    *   `RECIPE_ID`: 要删除的食谱的数字 ID。
    *   删除前会有确认提示。
    *   示例: `recipe-cli --recipe-delete 101`

*(请注意：上述命令和参数基于代码分析，建议在程序内运行 `recipe-cli --help` 以获取最准确和完整的命令列表及其用法。)*

### 3. 未来功能展望
*   食谱分类与标签管理
*   批量导入/导出食谱 (例如从 JSON, CSV 文件)
*   数据备份与恢复机制

## 六、配置选项说明

*   **数据存储路径**：
    *   用户数据: 默认存储在程序运行目录下的 `users.json` 文件。
    *   食谱数据: 默认存储在程序运行目录下的 `recipes.json` 文件。
    *   餐厅数据: 默认存储在程序运行目录下的 `restaurants.json` 文件 (餐厅相关功能目前未完全通过CLI暴露)。
    *   当前版本不支持通过配置文件或环境变量修改这些路径。

## 七、API 参考文档

目前项目主要通过命令行界面进行交互。如果未来计划提供 HTTP API 或库 API，将在此处详细说明。

## 八、故障排除与常见问题解答 (FAQ)

*   **Q1: 编译失败，提示找不到 `CMakeLists.txt`?**
    *   A: 请确保您在执行 `cmake ..` 命令时，当前目录是 `build` 目录，并且 `build` 目录与 `CMakeLists.txt` 文件所在的项目的根目录是同级。

*   **Q2: 运行程序时提示“命令未找到”或类似错误 (例如 `./recipe-cli: No such file or directory`)?**
    *   A: 请检查可执行文件 `recipe-cli` 是否已成功生成在 `build` 目录中。如果已生成，请确保您在正确的目录下执行它。
        *   如果您在 `build` 目录中，使用 `./recipe-cli`。
        *   如果您在项目根目录中，使用 `./build/recipe-cli`。

*   **Q3: 如何查看所有可用的命令？**
    *   A: 运行 `recipe-cli --help` 或 `recipe-cli -h`。

*   **Q4: 添加食谱时，如果名称或步骤包含空格怎么办？**
    *   A: 对于 `--recipe-add` 命令，所有信息都是通过交互式提示输入的，程序会处理包含空格的输入。对于其他接受参数的命令，如 `--recipe-search "Chicken Soup"`，如果参数值包含空格，请用双引号 `"` 将其括起来。

## 九、参与贡献指南

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

## 十、版本历史与更新日志

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
    *   数据通过 JSON 文件持久化。
    *   使用 cxxopts进行命令行参数解析。
    *   包含基本的 CMake 构建系统和 Google Test 单元测试框架。

## 十一、许可证信息

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

## 十二、联系方式与支持渠道

*   **项目维护者邮箱**：simazhangyu@gmail.com (来自原 README)
*   **GitHub Issues**：如果您有任何问题、BUG报告或功能建议，请通过项目仓库的 [Issues](https://github.com/ivekasy/IntelligentRecipeManagementSystem/issues) 页面提交。

---

感谢您使用 IntelligentRecipeManagementSystem (RecipeCliApp)！