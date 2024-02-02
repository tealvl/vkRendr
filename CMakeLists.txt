cmake_minimum_required(VERSION 3.26)

project(engine VERSION 0.1 LANGUAGES CXX)

add_subdirectory(dependencies/Vulkan-Hpp)
add_subdirectory(dependencies/Vulkan-Hpp/glm)
add_subdirectory(dependencies/Vulkan-Hpp/glfw)

add_executable(engine
    src/main.cpp
    src/HelloTriangleApplication.cpp
)

find_package(Vulkan REQUIRED)
target_link_libraries(engine PRIVATE Vulkan::Vulkan)

# Указываем пути к заголовочным файлам glm и glfw
target_include_directories(engine
    PRIVATE dependencies/Vulkan-Hpp/glm
    PRIVATE dependencies/Vulkan-Hpp/glfw/include
     PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Указываем путь к библиотекам glfw
target_link_directories(engine 
PRIVATE dependencies/Vulkan-Hpp/glfw/src)

# Линкуем сами библиотеки GLFW
target_link_libraries(engine 
PRIVATE glfw)