# CSC3002 游戏框架

## 项目概览
- 基于 C++17 构建，集成 SFML 3、Box2D 与 nlohmann-json
- 采用场景驱动架构，解耦渲染、输入、物理与资源管理
- 通过 INI 与 JSON 配置即可调整参数，无需重新编译
- 提供单元测试脚手架与示例，便于扩展各子系统

## 目录结构
```
CMakeLists.txt         # 构建配置，使用 FetchContent 拉取第三方依赖
assets/                # 运行期使用的美术、音频等资源
config/                # INI 与 JSON 配置文件（engine.ini、场景数据等）
src/main.cpp           # 程序入口与引擎初始化
src/engine/            # 核心系统：Display、EventSys、GameInput
src/loader/            # ConfigLoader（INI）与 ResourceLoader（JSON）
src/objects/           # 游戏对象基类与场景管理
src/include/           # 模块间共享的公共头文件
src/test/              # 单元与集成测试示例入口
build/                 # CMake 生成的构建产物
```

## 核心模块
- **Display (`src/engine/Display.cpp`)**：封装 SFML 窗口创建、帧清屏与呈现，并通过 `ConfigLoader` 读取显示参数。
- **EventSys (`src/engine/EventSys.cpp`)**：基于优先队列的即时/定时事件分发器，驱动任务系统顺序执行。
- **GameInput (`src/engine/GameInput.cpp`)**：统一键鼠轮询接口，提供逐键状态机与可选窗口相对坐标。
- **ConfigLoader (`src/loader/ConfigLoader.cpp`)**：轻量级 INI 解析器，自动推断整数、浮点、布尔、字符串及空值。
- **ResourceLoader (`src/loader/ResourceLoader.cpp`)**：JSON 场景加载器，提供标量读取与对象数组辅助方法（`getObjKeys`、`getObjResources`）。
- **BaseObj (`src/objects/GameObj.cpp`)**：对象生命周期辅助工具，支持事件注册与基于 `EventSys` 的绘制调度。
- **Scene (`src/objects/Scene.cpp`)**：负责 Box2D 世界初始化、资源驱动的对象构建、更新循环与渲染挂载点。

## 场景驱动开发流程
1. **手动构建场景**：为菜单、关卡等需求派生具体 `Scene` 类，场景持有自身资源与物理世界。
2. **资源初始化**：在 `Scene::init` 中调用 `ResourceLoader` 并读取 JSON 中的 `objKeys` 与对象数组，用以实例化并配置实体。
3. **生命周期约束**：仅允许 Scene 触发对象的逐帧 `update` 与 `render`，主循环禁止直接调用其他模块的更新函数。
4. **优先注册任务**：所有需在主循环执行的函数（渲染、物理回调、延迟销毁等）必须注册到 `EventSys`（即时或定时），避免对象删除后仍被调用。
5. **安全拆卸**：通过定时事件安排删除，确保相关回调先于对象释放执行。
6. **状态机推荐**：复杂行为对象应实现自定义状态机，并在 Scene 的调度流程中驱动。

## 配置与资源
- `config/engine.ini`
  - `[Display]`：窗口宽高、帧率上限、窗口标题等。
  - `[Engine]`：`DeltaTime`，用于模拟与调度。
  - `[Path]`：场景配置路径（如初始场景的 `MenuPath`）。
- `config/*.json`
  - `ResourceLoader` 支持扁平字典（参见 `flat_example.json`）与嵌套结构（参见 `example.json`）。
  - 使用 `objKeys` 声明要遍历的对象集合，每个集合中的对象可自定义键值。
- 除永久常量外请优先用配置文件注入参数，避免硬编码。
- 使用绝对路径或统一基目录（如 `ConfigLoader::setBaseDir`）以免路径漂移。

## 构建与运行
### 环境依赖
- CMake 3.28+
- 支持 C++17 的编译器（MSVC 2022、Clang 15+、GCC 11+）
- Git（供 FetchContent 克隆 SFML、Box2D、nlohmann-json）

### 配置构建目录
```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```
如使用多配置生成器，可追加 `-G "Visual Studio 17 2022"`。

### 编译
```powershell
cmake --build build --config Release
```
产物：
- 可执行文件：`build/bin/game.exe`
- 库文件：`build/lib`

### 运行
```powershell
build\bin\game.exe
```
确保当前工作目录包含 `config/` 与 `assets/`，以保证运行期读取资源。

## 测试
- 示例测试入口位于 `src/test/`（涵盖 SFML、Box2D、ConfigLoader、ResourceLoader、EventSys、KeyRead 等）。
- 若需启用特定测试，可在 `CMakeLists.txt` 中取消相应 `add_executable` 注释后重新构建。
- 建议扩展子系统时同步编写单元/集成测试，并通过 `ctest` 或直接执行测试程序验证。

## 编码规范
- **Scene 统一管理**：场景负责对象、资源与物理，不要在主循环中绕过 Scene 直接更新或绘制。
- **任务系统优先**：所有周期或延迟工作都要注册到 `EventSys`，避免在主循环或临时计时器中直接调用函数。
- **避免魔法数**：除确认永不变动的常量外，全部参数请通过 INI/JSON 配置提供。
- **资源集中管理**：使用 `ResourceLoader` 提供构造参数与资源路径，集中掌控生命周期。
- **物理集成**：在 Scene 初始化时创建 Box2D 物体，并在 `Scene::update` 中步进后再执行业务逻辑。
- **状态可控**：为复杂对象实现明确状态机，保证更新过程可预测、可调试。

## 协作与 AI 使用
- 修改前请充分阅读框架代码，仅在完全理解的模块内扩展。
- 若无明确注释开放扩展（如 `GameInput.cpp` 第 10 行），AI 辅助修改需限制在 `GameObj` 与 `Scene`。
- 跨模块修改前先与对应负责人沟通，遵守既定接口。
- 删除对象时通过任务系统注册定时事件，避免回调访问已释放的内存。