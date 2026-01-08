cmake_minimum_required(VERSION 3.10)
project(AutoAim)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
)

add_library(core
    src/LightBarDetector.cpp
    src/AutoAimSystem.cpp
    src/ArmorMatcher.cpp
)

target_link_libraries(core ${OpenCV_LIBS})

add_executable(autoaim src/main.cpp)
target_link_libraries(autoaim core)
