cmake_minimum_required(VERSION 3.28)
project(CSC3002_GroupProject)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
FetchContent_MakeAvailable(SFML box2d)

add_executable(SFML_test src/test/SFML_test.cpp)
target_compile_features(SFML_test PRIVATE cxx_std_17)
target_link_libraries(SFML_test PRIVATE
    SFML::Graphics
    SFML::Audio
    SFML::Window
    SFML::System
)

add_executable(box2d_test src/test/box2d_test.cpp)
target_compile_features(box2d_test PRIVATE cxx_std_17)
target_link_libraries(box2d_test PRIVATE
    box2d::box2d
)

add_executable(CL_test src/test/CL_test.cpp src/loader/ConfigLoader.cpp)
target_compile_features(CL_test PRIVATE cxx_std_17)
target_link_directories(CL_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/loader
)