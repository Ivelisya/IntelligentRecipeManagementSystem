# 智能食谱管理系统 v3.0 开发计划

## 项目目标

实现一个功能强大、用户体验卓越的智能食谱管理系统 v3.0。

## 核心功能点

1.  **结构化食谱数据管理：**
    *   **数据模型扩展：** 确保每条食谱包含：
        *   菜品名称
        *   详细食材列表（含用量）
        *   完整烹饪步骤
        *   预计烹饪时间
        *   难度级别
        *   自定义分类标签 (如菜系、口味、饮食限制等)
    *   **持久化存储：** 支持食谱数据的JSON格式持久化存储与高效加载。
2.  **高性能检索引擎：**
    *   **索引构建：** 使用C++标准库（如 `std::map` 或 `std::unordered_map`）为菜名、食材、标签等关键字段建立索引。
    *   **查询功能：**
        *   支持通过菜名、食材、标签等多种条件进行查询。
        *   支持精确查询。
        *   支持基于关键词匹配的模糊查询。
    *   **性能要求：** 确保在大数据量下的快速响应。
3.  **高度交互式命令行界面 (CLI)：**
    *   **用户体验提升：**
        *   直观的菜单导航。
        *   分步操作引导。
        *   智能问答方式（可选）。
        *   清晰的指令提示。
        *   上下文相关的帮助信息。
        *   智能命令补全功能 (可选/后期优化)。
    *   **信息展示：** 清晰、易读、结构化的食谱信息展示。
4.  **友好的错误处理机制：**
    *   在用户输入无效或操作不当时，提供明确的错误原因。
    *   提供修正建议。
5.  **版本与文档：**
    *   项目版本更新至 3.0。
    *   同步更新 `README.md` 文件以反映新功能和变更。

## 详细计划步骤

### 阶段一：数据模型与持久化层增强

1.  **定义新的/扩展的食谱数据结构：**
    *   在 `Recipe` 类中添加或修改成员变量：
        *   `std::vector<IngredientItem>` 用于食材列表，其中 `IngredientItem` 包含 `name` (string) 和 `quantity` (string)。
        *   `std::vector<std::string>` 用于烹饪步骤。
        *   `std::vector<std::string>` 用于自定义分类标签。
    *   **Mermaid - 食谱数据结构图示:**
        ```mermaid
        classDiagram
          Recipe <|-- IngredientItem
          Recipe {
            +string id
            +string name
            +vector~IngredientItem~ ingredients
            +vector~string~ steps
            +int cookingTimeMinutes
            +string difficulty
            +vector~string~ tags
            +string imageUrl (optional)
            +string nutritionalInfo (optional)
            +to_json()
            +from_json()
          }
          IngredientItem {
            +string name
            +string quantity
          }
        ```
2.  **更新JSON序列化/反序列化逻辑：**
    *   修改 `Recipe` 类中的 `to_json` 和 `from_json` 方法以支持新的数据结构。
3.  **更新持久化仓库：**
    *   修改 `JsonRecipeRepository` 以正确加载和保存包含新字段的食谱数据。
    *   测试数据加载和保存的正确性。

### 阶段二：检索引擎设计与实现

1.  **设计索引结构：**
    *   为菜名、每个食材的名称、每个标签设计索引。
    *   考虑使用 `std::map<std::string, std::set<RecipeID>>` 或 `std::unordered_map<std::string, std::set<RecipeID>>`。
    *   **Mermaid - 检索引擎核心结构图示:**
        ```mermaid
        graph LR
          subgraph IndexManager
            RecipeNameIndex["菜名索引 (map<string, set<RecipeID>>)"]
            IngredientIndex["食材索引 (map<string, set<RecipeID>>)"]
            TagIndex["标签索引 (map<string, set<RecipeID>>)"]
          end

          RecipeDataSource["食谱数据源 (vector<Recipe>)"] -->|构建索引| IndexManager
          UserQuery["用户查询 (菜名, 食材, 标签)"] -->|查询处理| IndexManager
          IndexManager -->|返回结果| SearchResults["查询结果 (vector<RecipeID>)"]
        ```
2.  **实现索引构建逻辑：**
    *   在 `RecipeManager` 或新的 `SearchService` 类中实现。
    *   当食谱数据加载或发生变更时，同步更新索引。
3.  **实现查询逻辑：**
    *   精确查询：直接在索引中查找。
    *   模糊查询 (关键词匹配)：对用户输入进行分词，在索引中查找，并处理多条件查询。
    *   实现将查询到的 `RecipeID` 转换为完整的 `Recipe` 对象列表。
4.  **集成到 `RecipeManager`：**
    *   `RecipeManager` 提供新的搜索接口。

### 阶段三：命令行界面 (CLI) 增强

1.  **设计新的CLI交互流程：**
    *   主菜单：提供清晰选项。
    *   分步引导：添加/更新食谱时逐项提示。
    *   搜索界面：允许用户选择搜索条件并输入内容。
2.  **实现CLI交互逻辑：**
    *   修改 `main.cpp` 或相关的CLI处理程序。
    *   指令提示与帮助：提供清晰提示和上下文帮助。
    *   食谱展示：格式化输出食谱详情。
3.  **智能命令补全 (可选/后期优化)。**

### 阶段四：错误处理与用户体验优化 (贯穿整个开发过程)

1.  **统一错误处理：** 使用异常处理机制。
2.  **提供明确的错误信息：** 清晰指出问题所在。
3.  **提供修正建议：** 尽可能给出修正建议。
4.  **测试各种错误场景。**

### 阶段五：版本更新与文档

1.  **更新项目版本号：** 更新为 `3.0.0`。
2.  **更新 `README.md`：**
    *   详细描述新功能和修改。
    *   更新使用指南和命令示例。
    *   说明检索引擎的架构（如果适用）。
    *   添加 v3.0.0 的更新日志。
3.  **代码注释和文档字符串：** 确保新代码有良好注释。

## 时间预估

(占位符 - 具体时间需根据实际情况评估)

*   阶段一：数据模型与持久化层增强 (预计耗时：X天)
*   阶段二：检索引擎设计与实现 (预计耗时：Y天)
*   阶段三：命令行界面 (CLI) 增强 (预计耗时：Z天)
*   阶段四：错误处理与用户体验优化 (贯穿)
*   阶段五：版本更新与文档 (预计耗时：W天)

## 潜在风险与应对

*   **检索引擎性能：** 初期按计划使用标准库，后续可根据性能测试结果优化。
*   **CLI交互复杂性：** “智能问答”和“命令补全”可分阶段实现，优先核心交互。
*   **向后兼容性：** 当前计划假设可以覆盖旧数据或从新版本开始。