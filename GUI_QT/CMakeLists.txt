cmake_minimum_required(VERSION 3.10)
project(HipEncryptorGUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

add_executable(gui main.cpp)

target_link_libraries(gui Qt5::Widgets)