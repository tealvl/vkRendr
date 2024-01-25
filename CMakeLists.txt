cmake_minimum_required(VERSION 3.26)

project(engine VERSION 0.1 LANGUAGES CXX)

add_subdirectory(dependencies/Vulkan-Hpp)

add_executable(engine
src/main.cpp
)