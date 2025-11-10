cmake_minimum_required(VERSION 3.28)
project(CSC3002_GroupProject)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 设置C++标准为C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 项目依赖SFML和Box2D和nlohmann-json
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG main
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(SFML box2d nlohmann_json)

# 定义配置加载器库
add_library(ConfigLib
    src/loader/ConfigLoader.cpp
)
target_include_directories(ConfigLib PUBLIC src/include)

# 定义资源加载器库
add_library(ResourceLib
    src/loader/ResourceLoader.cpp
)
target_include_directories(ResourceLib PUBLIC src/include)
target_link_libraries(ResourceLib PUBLIC
    nlohmann_json::nlohmann_json
)

# 定义输入读取库
add_library(GameInputLib
    src/engine/GameInput.cpp
)
target_include_directories(GameInputLib PUBLIC src/include)
target_link_libraries(GameInputLib PUBLIC
    SFML::Window
    SFML::Graphics
    SFML::System
)

# 定义事件系统库
add_library(EventSysLib
    src/engine/EventSys.cpp
)
target_include_directories(EventSysLib PUBLIC src/include)
target_link_libraries(EventSysLib PUBLIC 
    SFML::System
)

# 定义显示库
add_library(DisplayLib
    src/engine/Display.cpp
)
target_include_directories(DisplayLib PUBLIC src/include)
target_link_libraries(DisplayLib PUBLIC 
    ConfigLib
    SFML::Graphics
)

# 定义游戏对象库
add_library(GameObjLib
    src/objects/GameObj.cpp
)
target_include_directories(GameObjLib PUBLIC src/include)
target_link_libraries(GameObjLib PUBLIC
    EventSysLib
    ResourceLib
    GameInputLib
    box2d::box2d
    SFML::Graphics
    SFML::Audio
)

# 定义游戏场景库
add_library(GameSceneLib
    src/objects/Scene.cpp
)
target_include_directories(GameSceneLib PUBLIC src/include)
target_link_libraries(GameSceneLib PUBLIC
    GameObjLib
    ResourceLib
    EventSysLib
    GameInputLib
    box2d::box2d
)

# 生成游戏主程序
add_executable(game src/main.cpp)
target_compile_features(game PRIVATE cxx_std_17)
target_link_libraries(game PRIVATE
    ConfigLib
    ResourceLib
    DisplayLib
    EventSysLib
    GameInputLib
    GameObjLib
    GameSceneLib
    SFML::Graphics
    SFML::Audio
    SFML::Window
    SFML::System
)


# # 测试 SFML
# add_executable(SFML_test src/test/SFML_test.cpp)
# target_compile_features(SFML_test PRIVATE cxx_std_17)
# target_link_libraries(SFML_test PRIVATE
#     SFML::Graphics
#     SFML::Audio
#     SFML::Window
#     SFML::System
# )

# # 测试 Box2D
# add_executable(box2d_test src/test/box2d_test.cpp)
# target_compile_features(box2d_test PRIVATE cxx_std_17)
# target_link_libraries(box2d_test PRIVATE
#     box2d::box2d
# )

# # 测试 ConfigLoader 
# add_executable(CL_test src/test/CL_test.cpp src/loader/ConfigLoader.cpp)
# target_compile_features(CL_test PRIVATE cxx_std_17)
# target_link_directories(CL_test PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/loader
# )

# 测试 ResourceLoader
add_executable(RL_test src/test/RL_test.cpp src/loader/ResourceLoader.cpp)
target_compile_features(RL_test PRIVATE cxx_std_17)
target_link_libraries(RL_test PRIVATE
    ResourceLib
)

# # 测试按键读取
# add_executable(KeyRead_test src/test/KeyRead_test.cpp)
# target_compile_features(KeyRead_test PRIVATE cxx_std_17)
# target_link_libraries(KeyRead_test PRIVATE
#     GameInputLib
#     SFML::Graphics
# )
