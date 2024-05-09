cmake_minimum_required(VERSION 3.26)

project(engine VERSION 0.1 LANGUAGES CXX)

add_subdirectory(dependencies/Vulkan-Hpp)
add_subdirectory(dependencies/Vulkan-Hpp/glm)
add_subdirectory(dependencies/Vulkan-Hpp/glfw)

add_executable(engine
    src/main.cpp
    src/HelloTriangleApplication.cpp
    src/renderer/test.cpp
    src/renderer/window.cpp
    src/renderer/camera.cpp
    src/renderer/transform.cpp
    src/renderer/instance.cpp
    src/renderer/debug.cpp
)

# Указываем пути к заголовочным файлам 
target_include_directories(engine
    PRIVATE dependencies/Vulkan-Hpp/glm
    PRIVATE dependencies/Vulkan-Hpp/glfw/include
    PRIVATE dependencies/Vulkan-Hpp/vulkan/
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer
    PRIVATE dependencies/stb
    PRIVATE dependencies/tinyobjloader


)

# Указываем путь к исходникам
target_link_directories(engine 
PRIVATE dependencies/Vulkan-Hpp/glfw/src
)


find_package(Vulkan REQUIRED)
# Линкуем библиотеки 
target_link_libraries(engine 
PRIVATE Vulkan::Vulkan
PRIVATE glfw
)