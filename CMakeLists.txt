cmake_minimum_required(VERSION 3.10)
project(AutoAim)

set(CMAKE_CXX_STANDARD 17)

# 查找 OpenCV
find_package(OpenCV REQUIRED)

# 包含头文件
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
)

# 编译核心库
add_library(core
    src/LightBarDetector.cpp
    src/AutoAimSystem.cpp
    src/ArmorMatcher.cpp
    src/NumberRecognizer.cpp
)

# 链接 OpenCV
target_link_libraries(core PRIVATE ${OpenCV_LIBS})

# 编译可执行程序
add_executable(autoaim src/main.cpp)

# 链接核心库
target_link_libraries(autoaim PRIVATE core)
